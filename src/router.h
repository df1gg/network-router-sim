#ifndef ROUTER_H
#define ROUTER_H

#include "common.h"

struct Router {
  struct Port ports[PORTS_COUNT];
  int mac_table[PORTS_COUNT];
};

void forward_packet(struct Router *router, struct Packet *pkt, int port_id);
void router_tock(struct Router *router);

#endif // ROUTER_H
