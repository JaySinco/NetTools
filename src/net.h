#pragma once
#include <iostream>
#include <iomanip>
#include <sstream>
#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <gflags/gflags.h>
#include <glog/logging.h>
#include "pcap.h"

#define ETHERNET_ADDRESS_LEN 6
#define IPV4_ADDRESS_LEN 4
#define ETHERNET_TYPE_IPv4 0x0800
#define ETHERNET_TYPE_IPv6 0x86DD
#define ETHERNET_TYPE_ARP  0x0806
#define ETHERNET_TYPE_RARP 0x8035
#define ARP_HARDWARE_TYPE_ETHERNET 1
#define ARP_REQUEST_OP  1
#define ARP_REPLY_OP    2
#define RARP_REQUEST_OP 3
#define RARP_REPLY_op   4
#define BROADCAST_IPv4_ADDR ip4_addr{0xff, 0xff, 0xff, 0xff}
#define PLACEHOLDER_IPv4_ADDR ip4_addr{0x0, 0x0, 0x0, 0x0}
#define BROADCAST_ETH_ADDR eth_addr{0xff, 0xff, 0xff, 0xff, 0xff, 0xff}
#define PLACEHOLDER_ETH_ADDR eth_addr{0x0, 0x0, 0x0, 0x0, 0x0, 0x0}

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

struct adapter_info {
    std::string name;
    std::string desc;
    ip4_addr ip;
    ip4_addr mask;
    ip4_addr gateway;
    eth_addr mac;
};

template <typename T>
std::string to_string(const T& v)
{
    std::ostringstream ss;
    ss << v;
    return ss.str();
}

bool get_adapter_info_by_ip4(const ip4_addr &ip, adapter_info &info);

bool send_arp(
    pcap_t *adhandle, u_short op,
    const eth_addr &sea, const ip4_addr &sia, const eth_addr &dea, const ip4_addr &dia);

bool ip4_from_string(const std::string &s, ip4_addr &addr);
ip4_addr ip4_from_win(const in_addr &waddr);
in_addr ip4_to_win(const ip4_addr &addr);

u_int operator&(const ip4_addr &a, const ip4_addr &b);
bool operator==(const ip4_addr &a, const ip4_addr &b);
bool operator!=(const ip4_addr &a, const ip4_addr &b);
std::ostream &operator<<(std::ostream &out, const in_addr &addr);
std::ostream &operator<<(std::ostream &out, const in6_addr &addr);
std::ostream &operator<<(std::ostream &out, const sockaddr *addr);
std::ostream &operator<<(std::ostream &out, const pcap_if_t *dev);
std::ostream &operator<<(std::ostream &out, const ip4_addr &addr);
std::ostream &operator<<(std::ostream &out, const eth_addr &addr);
std::ostream &operator<<(std::ostream &out, const eth_ip4_arp *arp_data);
std::ostream &print_packet(std::ostream &out, const pcap_pkthdr *header, const u_char *pkt_data);
