#ifndef HOST_H
#define HOST_H

#include "common.h"

struct Router;

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

int host_get_mac_from_cache(struct Host *host, int target_ip);
void host_push_to_wire(struct Router *router, struct Host *host,
                       struct Packet *pkt);
void host_receive(struct Router *router, struct Host *host,
                  struct Packet rx_pkt);
void host_tock(struct Router *router, struct Host *host);

#endif // HOST_H
