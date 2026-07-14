#include "host.h"
#include "router.h"
#include <stdio.h>

int host_get_mac_from_cache(struct Host *host, int target_ip) {
  for (int i = 0; i < host->arp_cache_count; i++) {
    if (host->arp_cache[i].ip == target_ip) {
      return host->arp_cache[i].mac;
    }
  }
  return -1;
}

void host_push_to_wire(struct Router *router, struct Host *host,
                       struct Packet *pkt) {
  if (pkt->type == PACKET_IP) {
    int dst_mac = host_get_mac_from_cache(host, pkt->dst_ip);
    if (dst_mac != -1) {
      pkt->dst_mac = dst_mac;
    } else {
      host->pending_packet = *pkt;
      host->has_pending = 1;

      pkt->type = PACKET_ARP_REQ;
      pkt->dst_mac = 255;
      snprintf(pkt->payload, PAYLOAD_SIZE, "ARP_REQ");
    }
  }

  router->ports[host->port_id].rx_buffer = *pkt;
  router->ports[host->port_id].has_rx_data = 1;
  printf("[%d/%d]: Send packet on wire %d port\n", pkt->src_ip, pkt->src_mac,
         host->port_id);
}

void host_receive(struct Router *router, struct Host *host,
                  struct Packet rx_pkt) {
  if (rx_pkt.type == PACKET_IP) {
    if (rx_pkt.dst_mac == host->mac) {
      printf("[%d/%d]: Received data packet from IP %d (MAC %d): %s\n",
             host->ip, host->mac, rx_pkt.src_ip, rx_pkt.src_mac,
             rx_pkt.payload);
    }
  } else if (rx_pkt.type == PACKET_ARP_REQ) {
    if (rx_pkt.dst_ip == host->ip) {
      printf("[%d/%d]: Got ARP request! Telling my MAC %d to IP %d\n", host->ip,
             host->mac, host->mac, rx_pkt.src_ip);

      struct Packet response;
      response.type = PACKET_ARP_RESP;
      response.src_ip = host->ip;
      response.src_mac = host->mac;
      response.dst_ip = rx_pkt.src_ip;
      response.dst_mac = rx_pkt.src_mac;
      snprintf(response.payload, PAYLOAD_SIZE, "ARP_REPLY");

      host_push_to_wire(router, host, &response);
    }
  } else if (rx_pkt.type == PACKET_ARP_RESP) {
    if (rx_pkt.dst_mac == host->mac) {
      printf("[%d/%d]: Got ARP reply! IP %d is at MAC %d. Saving to cache...\n",
             host->ip, host->mac, rx_pkt.src_ip, rx_pkt.src_mac);

      if (host->arp_cache_count < ARP_CACHE_SIZE) {
        host->arp_cache[host->arp_cache_count].ip = rx_pkt.src_ip;
        host->arp_cache[host->arp_cache_count].mac = rx_pkt.src_mac;
        host->arp_cache_count++;
      }

      if (host->has_pending) {
        printf("[%d/%d]: Found pending packet. Sending it now!\n", host->ip,
               host->mac);

        host->pending_packet.dst_mac = rx_pkt.src_mac;
        host->pending_packet.type = PACKET_IP;
        host->has_pending = 0;

        host_push_to_wire(router, host, &host->pending_packet);
      }
    }
  }
}

void host_tock(struct Router *router, struct Host *host) {
  if (router->ports[host->port_id].has_tx_data == 1) {
    struct Packet rx_pkt = router->ports[host->port_id].tx_buffer;
    router->ports[host->port_id].has_tx_data = 0;
    if (rx_pkt.type == PACKET_ARP_REQ)
      printf("[%d/%d]: New ARP request packet from IP %d (%d MAC)\n",
             rx_pkt.dst_ip, rx_pkt.dst_mac, rx_pkt.src_ip, rx_pkt.src_mac);
    else if (rx_pkt.type == PACKET_ARP_RESP)
      printf("[%d/%d]: New ARP response packet from IP %d (%d MAC)\n",
             rx_pkt.dst_ip, rx_pkt.dst_mac, rx_pkt.src_ip, rx_pkt.src_mac);
    else if (rx_pkt.type == PACKET_IP)
      printf("[%d/%d]: New packet from IP %d (%d MAC) with data: %s\n",
             rx_pkt.dst_ip, rx_pkt.dst_mac, rx_pkt.src_ip, rx_pkt.src_mac,
             rx_pkt.payload);

    host_receive(router, host, rx_pkt);
  }
}
