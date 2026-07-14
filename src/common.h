#ifndef COMMON_H
#define COMMON_H

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

#endif // COMMON_H
