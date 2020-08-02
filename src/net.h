#pragma once
#include <iostream>
#include <iomanip>
#include <gflags/gflags.h>
#include <glog/logging.h>
#include "pcap.h"

struct eth_addr {
    u_char b1, b2, b3, b4, b5, b6;
};

struct ip4_addr {
    u_char b1, b2, b3, b4;
};

struct ethernet_header {
    eth_addr dea;      // Destination address
    eth_addr sea;      // Source address
    u_short  eth_type; // Ethernet type
};

struct eth_ip4_arp {
    u_short  hw_type;   // Hardware type
    u_short  proto;     // Protocol
    u_char   hw_len;    // Hardware address length
    u_char   proto_len; // Protocol address length
    u_short  op;        // Operation code
    eth_addr sea;       // Source ethernet address
    ip4_addr sia;       // Source ip address
    eth_addr dea;       // Destination ethernet address
    ip4_addr dia;       // Destination ip address
};

struct ip4_header {
    u_char   ver_ihl;        // Version (4 bits) + Internet header length (4 bits)
    u_char   tos;            // Type of service
    u_short  tlen;           // Total length
    u_short  identification; // Identification
    u_short  flags_fo;       // Flags (3 bits) + Fragment offset (13 bits)
    u_char   ttl;            // Time to live
    u_char   proto;          // Protocol
    u_short  crc;            // Header checksum
    ip4_addr sia;            // Source address
    ip4_addr dia;            // Destination address
};

struct udp_header {
    u_short sport; // Source port
    u_short dport; // Destination port
    u_short len;   // Datagram length
    u_short crc;   // Checksum
};

std::ostream &operator<<(std::ostream &out, const in_addr &addr);
std::ostream &operator<<(std::ostream &out, const in6_addr &addr);
std::ostream &operator<<(std::ostream &out, const sockaddr *addr);
std::ostream &operator<<(std::ostream &out, const pcap_if_t *dev);
std::ostream &operator<<(std::ostream &out, const ip4_addr &addr);
std::ostream &operator<<(std::ostream &out, const eth_addr &addr);
std::ostream &operator<<(std::ostream &out, const eth_ip4_arp *arp_data);
std::ostream &print_packet(std::ostream &out, const pcap_pkthdr *header, const u_char *pkt_data);
