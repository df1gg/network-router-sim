#include "router.h"
#include <stdio.h>

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
