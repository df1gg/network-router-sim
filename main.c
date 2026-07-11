#include <stdio.h>
#include <string.h>

#define PAYLOAD_SIZE 256
#define HOST_NAME_SIZE 32
#define PORTS_COUNT 4

struct Packet {
  int src_ip;
  int dst_ip;
  int src_mac;
  int dst_mac;
  char payload[PAYLOAD_SIZE];
};

struct Port {
  struct Packet rx_buffer;
  int is_data;
};

struct Router {
  struct Port ports[PORTS_COUNT];
  int mac_table[PORTS_COUNT];
};

struct Host {
  char name[HOST_NAME_SIZE];
  int ip;
  int mac;
  int subnet_mask;
  int gateway_ip;
};

void host_push_to_wire(struct Router *router, int port_id, struct Packet pkt) {
  router->ports[port_id].rx_buffer = pkt;
  router->ports[port_id].is_data = 1;
  printf("[Host]: Send packet on wire %d port\n", port_id);
}

void forward_packet(struct Router *router, struct Packet *pkt) {
  int founded_port = -1;

  for (int i = 0; i < PORTS_COUNT; i++) {
    if (pkt->dst_mac == router->mac_table[i]) {
      founded_port = i;
      break;
    }
  }

  if (founded_port != -1) {
    printf("[Router]: Received packet to MAC: %d, with data: %s\n",
           pkt->dst_mac, pkt->payload);
  } else {
    printf("[Router]: Port not found!\n");
  }
}

void router_tock(struct Router *router) {
  for (int i = 0; i < PORTS_COUNT; i++) {
    if (router->ports[i].is_data == 1) {
      struct Packet inc_packet = router->ports[i].rx_buffer;
      router->ports[i].is_data = 0;

      printf("[Router]: Detected packet on %d port. Forward to dest IP...\n",
             i);

      forward_packet(router, &inc_packet);
      break;
    }
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

  struct Router my_router;
  my_router.mac_table[0] = 0;
  my_router.mac_table[1] = computer_1.mac;
  my_router.mac_table[2] = computer_2.mac;
  my_router.mac_table[3] = 0;

  struct Packet packet;
  packet.src_ip = computer_1.ip;
  packet.src_mac = computer_1.mac;
  packet.dst_ip = 103;
  packet.dst_mac = 2222;
  snprintf(packet.payload, PAYLOAD_SIZE, "Hello from %s!", computer_1.name);
  host_push_to_wire(&my_router, 1, packet);
  router_tock(&my_router);

  return 0;
}
