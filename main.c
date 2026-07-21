#include "src/common.h"
#include "src/host.h"
#include "src/router.h"
#include <stdio.h>
#include <string.h>

int main(void) {
  struct Host computer_1;
  strncpy(computer_1.name, "Arch", HOST_NAME_SIZE);
  computer_1.ip = MAKE_IP(192, 168, 1, 102);
  computer_1.mac = 1111;
  computer_1.gateway_ip = MAKE_IP(192, 168, 1, 101);
  computer_1.subnet_mask = 24;

  struct Host computer_2;
  strncpy(computer_2.name, "NixOs", HOST_NAME_SIZE);
  computer_2.ip = MAKE_IP(192, 168, 1, 103);
  computer_2.mac = 2222;
  computer_2.gateway_ip = MAKE_IP(192, 168, 1, 101);
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

  host_push_to_wire(&my_router, &computer_1, &packet);
  router_tock(&my_router);
  host_tock(&my_router, &computer_2);
  router_tock(&my_router);
  host_tock(&my_router, &computer_1);
  router_tock(&my_router);
  host_tock(&my_router, &computer_2);

  return 0;
}
