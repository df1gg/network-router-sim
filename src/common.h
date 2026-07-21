#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>

#define PAYLOAD_SIZE 256
#define HOST_NAME_SIZE 32
#define PORTS_COUNT 4
#define ARP_CACHE_SIZE 10

#define MAKE_IP(a, b, c, d)                                                    \
  (((uint32_t)(a) << 24) | ((uint32_t)(b) << 16) | ((uint32_t)(c) << 8) |      \
   (uint32_t)(d))

#define PRINT_IP_FMT "%u.%u.%u.%u"
#define PRINT_IP_ARGS(ip)                                                      \
  ((ip) >> 24) & 0xFF, ((ip) >> 16) & 0xFF, ((ip) >> 8) & 0xFF, (ip) & 0xFF

enum PacketType { PACKET_IP, PACKET_ARP_REQ, PACKET_ARP_RESP };

struct Packet {
  enum PacketType type;
  uint32_t src_ip;
  uint32_t dst_ip;
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

#endif // COMMON_H
