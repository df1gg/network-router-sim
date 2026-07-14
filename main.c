#include <stdio.h>
#include <string.h>

#define PAYLOAD_SIZE 256
#define HOST_NAME_SIZE 32
#define PORTS_COUNT 4
#define ARP_CACHE_SIZE 10

enum PacketType { PACKET_IP, PACKET_ARP_REQ, PACKET_ARP_RESP };

struct Packet {
  enum PacketType type;
  int src_ip;
  int dst_ip;
  int src_mac;
  int dst_mac;
  char payload[PAYLOAD_SIZE];
};

struct Port {
  struct Packet rx_buffer;
  int has_rx_data;

  struct Packet tx_buffer;
  int has_tx_data;
};

struct Router {
  struct Port ports[PORTS_COUNT];
  int mac_table[PORTS_COUNT];
};

struct ArpEntry {
  int ip;
  int mac;
};

struct Host {
  char name[HOST_NAME_SIZE];
  int ip;
  int mac;
  int subnet_mask;
  int gateway_ip;
  int port_id;

  struct ArpEntry arp_cache[ARP_CACHE_SIZE];
  int arp_cache_count;

  struct Packet pending_packet;
  int has_pending;
};

int host_get_mac_from_cache(struct Host *host, int target_ip) {
  for (int i = 0; i < host->arp_cache_count; i++) {
    if (host->arp_cache[i].ip == target_ip) {
      return host->arp_cache[i].mac;
    }
  }
  return -1;
}

void host_push_to_wire(struct Router *router, struct Host *host, int port_id,
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
    }
  }

  router->ports[port_id].rx_buffer = *pkt;
  router->ports[port_id].has_rx_data = 1;
  printf("[Host mac: %d]: Send packet on wire %d port\n", pkt->src_mac,
         port_id);
}

void forward_packet(struct Router *router, struct Packet *pkt, int port_id) {
  if (pkt->dst_mac == 255) {
    for (int i = 0; i < PORTS_COUNT; i++) {
      if (port_id == i)
        continue;
      if (router->mac_table[i] == 0)
        continue;
      router->ports[i].tx_buffer = *pkt;
      router->ports[i].has_tx_data = 1;
    }
    return;
  }

  int target_port = -1;
  for (int i = 0; i < PORTS_COUNT; i++) {
    if (pkt->dst_mac == router->mac_table[i]) {
      target_port = i;
      break;
    }
  }

  if (target_port != -1) {
    router->ports[target_port].tx_buffer = *pkt;
    router->ports[target_port].has_tx_data = 1;
    printf("[Router]: Packet forwarded to physical Port %d\n", target_port);
  } else {
    printf("[Router]: Flood or drop! MAC not found.\n");
  }
}

void host_receive(struct Router *router, struct Host *host,
                  struct Packet rx_pkt) {
  if (rx_pkt.type == PACKET_IP) {
    if (rx_pkt.dst_mac == host->mac) {
      printf("[%s]: Received data packet from IP %d (MAC %d): %s\n", host->name,
             rx_pkt.src_ip, rx_pkt.src_mac, rx_pkt.payload);
    }
  } else if (rx_pkt.type == PACKET_ARP_REQ) {
    if (rx_pkt.dst_ip == host->ip) {
      printf("[%s]: Got ARP request! Telling my MAC %d to IP %d\n", host->name,
             host->mac, rx_pkt.src_ip);

      struct Packet response;
      response.type = PACKET_ARP_RESP;
      response.src_ip = host->ip;
      response.src_mac = host->mac;
      response.dst_ip = rx_pkt.src_ip;
      response.dst_mac = rx_pkt.src_mac;
      snprintf(response.payload, PAYLOAD_SIZE, "ARP_REPLY");

      host_push_to_wire(router, host, host->port_id, &response);
    }
  } else if (rx_pkt.type == PACKET_ARP_RESP) {
    if (rx_pkt.dst_mac == host->mac) {
      printf("[%s]: Got ARP reply! IP %d is at MAC %d. Saving to cache...\n",
             host->name, rx_pkt.src_ip, rx_pkt.src_mac);

      if (host->arp_cache_count < ARP_CACHE_SIZE) {
        host->arp_cache[host->arp_cache_count].ip = rx_pkt.src_ip;
        host->arp_cache[host->arp_cache_count].mac = rx_pkt.src_mac;
        host->arp_cache_count++;
      }

      if (host->has_pending) {
        printf("[%s]: Found pending packet. Sending it now!\n", host->name);

        host->pending_packet.dst_mac = rx_pkt.src_mac;
        host->pending_packet.type = PACKET_IP;
        host->has_pending = 0;

        host_push_to_wire(router, host, host->port_id, &host->pending_packet);
      }
    }
  }
}

void router_tock(struct Router *router) {
  for (int i = 0; i < PORTS_COUNT; i++) {
    if (router->ports[i].has_rx_data == 1) {
      struct Packet inc_packet = router->ports[i].rx_buffer;
      router->ports[i].has_rx_data = 0;

      printf("[Router]: Detected packet on %d port. Forward to dest physical "
             "Port...\n",
             i);

      forward_packet(router, &inc_packet, i);
      break;
    }
  }
}

void host_tock(struct Router *router, struct Host *host) {
  if (router->ports[host->port_id].has_tx_data == 1) {
    struct Packet rx_pkt = router->ports[host->port_id].tx_buffer;
    router->ports[host->port_id].has_tx_data = 0;
    printf("[Host mac: %d] New packet from %d MAC with data: %s\n",
           rx_pkt.dst_mac, rx_pkt.src_mac, rx_pkt.payload);
    host_receive(router, host, rx_pkt);
  }
}

int main(void) {
  struct Host computer_1;
  strncpy(computer_1.name, "Arch", HOST_NAME_SIZE);
  computer_1.ip = 102;
  computer_1.mac = 1111;
  computer_1.gateway_ip = 101;
  computer_1.subnet_mask = 24;

  struct Host computer_2;
  strncpy(computer_2.name, "NixOs", HOST_NAME_SIZE);
  computer_2.ip = 103;
  computer_2.mac = 2222;
  computer_2.gateway_ip = 101;
  computer_2.subnet_mask = 24;

  computer_1.arp_cache_count = 0;
  computer_1.has_pending = 0;
  computer_2.arp_cache_count = 0;
  computer_2.has_pending = 0;

  struct Router my_router;
  memset(&my_router, 0, sizeof(struct Router));
  my_router.mac_table[0] = 0;
  my_router.mac_table[1] = computer_1.mac;
  computer_1.port_id = 1;
  my_router.mac_table[2] = computer_2.mac;
  computer_2.port_id = 2;
  my_router.mac_table[3] = 0;

  struct Packet packet;
  packet.type = PACKET_IP;
  packet.src_ip = computer_1.ip;
  packet.src_mac = computer_1.mac;
  packet.dst_ip = computer_2.ip;
  packet.dst_mac = 0;
  snprintf(packet.payload, PAYLOAD_SIZE, "Hello from %s!", computer_1.name);

  host_push_to_wire(&my_router, &computer_1, computer_1.port_id, &packet);
  router_tock(&my_router);
  host_tock(&my_router, &computer_2);
  router_tock(&my_router);
  host_tock(&my_router, &computer_1);
  router_tock(&my_router);
  host_tock(&my_router, &computer_2);

  return 0;
}
