#include <stdio.h>
#include <string.h>

#define PAYLOAD_SIZE 256
#define PORTS_COUNT 4

struct Packet {
  int dest_ip;
  int dest_mac;
  char payload[PAYLOAD_SIZE];
};

struct Router {
  int mac_table[PORTS_COUNT];
};

void forward_packet(struct Router *router, struct Packet *pkt) {
  printf("[Router]: Received packet for MAC: %d. Checking ports...\n",
         pkt->dest_mac);

  int found_port = -1;

  for (int i = 0; i < PORTS_COUNT; i++) {
    if (router->mac_table[i] == pkt->dest_mac) {
      found_port = i;
      break;
    }
  }

  if (found_port != -1) {
    printf("[Router]: Sending packet to port: %d (Data: %s)...\n", found_port,
           pkt->payload);
  } else {
    printf("[Router]: Port not found!\n");
  }
}

int main(void) {
  struct Router my_router;
  my_router.mac_table[0] = 0;
  my_router.mac_table[1] = 1111;
  my_router.mac_table[2] = 2222;
  my_router.mac_table[3] = 0;

  struct Packet my_packet;
  my_packet.dest_ip = 102;
  my_packet.dest_mac = 2222;
  strncpy(my_packet.payload, "Hello from first computer!", PAYLOAD_SIZE);

  forward_packet(&my_router, &my_packet);

  return 0;
}
