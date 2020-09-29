#pragma once
#include <string>
#include <iostream>
#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <gflags/gflags.h>
#include <glog/logging.h>
#include "common.h"

#define ETHERNET_ADDRESS_LEN 6
#define IPV4_ADDRESS_LEN 4

#define ETHERNET_TYPE_IPv4 0x0800
#define ETHERNET_TYPE_IPv6 0x86DD
#define ETHERNET_TYPE_ARP 0x0806
#define ETHERNET_TYPE_RARP 0x8035

#define IPv4_TYPE_ICMP 1
#define IPv4_TYPE_TCP 6
#define IPv4_TYPE_UDP 17

#define ARP_HARDWARE_TYPE_ETHERNET 1
#define ARP_REQUEST_OP 1
#define ARP_REPLY_OP 2
#define RARP_REQUEST_OP 3
#define RARP_REPLY_op 4

#define ICMP_TYPE_PING_REPLY 0
#define ICMP_TYPE_ERROR_UNREACHABLE 3
#define ICMP_TYPE_ERROR_SOURCE_CLOSED 4
#define ICMP_TYPE_ERROR_REDIRECTION 5
#define ICMP_TYPE_PING_ASK 8
#define ICMP_TYPE_ROUTER_NOTICE 9
#define ICMP_TYPE_ROUTER_REQUEST 10
#define ICMP_TYPE_ERROR_TIMEOUT 11
#define ICMP_TYPE_ERROR_PARAMETER 12
#define ICMP_TYPE_TIMESTAMP_ASK 13
#define ICMP_TYPE_TIMESTAMP_REPLY 14
#define ICMP_TYPE_INFO_ASK 15
#define ICMP_TYPE_INFO_REPLY 16
#define ICMP_TYPE_NETMASK_ASK 17
#define ICMP_TYPE_NETMASK_REPLY 18

#define DNS_TYPE_CLASS_INTERNET_ADDR 1
#define DNS_TYPE_A 1
#define DNS_TYPE_NS 2
#define DNS_TYPE_CNAME 5
#define DNS_TYPE_PTR 12
#define DNS_TYPE_HINFO 13
#define DNS_TYPE_MX 15
#define DNS_TYPE_AXFR 252
#define DNS_TYPE_ANY 255
#define DNS_OPCODE_QUERY_STANDARD 0
#define DNS_OPCODE_QUERY_REVERSE 1
#define DNS_OPCODE_QUERY_SERVER_STATUS 2
#define DNS_RCODE_NO_ERROR 0
#define DNS_RCODE_NAME_ERROR 3

#define BROADCAST_IPv4_ADDR \
    ip4_addr { 0xff, 0xff, 0xff, 0xff }
#define PLACEHOLDER_IPv4_ADDR \
    ip4_addr { 0x0, 0x0, 0x0, 0x0 }
#define BROADCAST_ETH_ADDR \
    eth_addr { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff }
#define PLACEHOLDER_ETH_ADDR \
    eth_addr { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 }

#define PTR_AFTER(p) (((char *)(p)) + sizeof(decltype(*(p))))
#define DELIMITER_LINE "\t------------------------\n"
#define DELIMITER_SUBLINE "\t+-+-+-+-+-+-+-+-+-+-+-+-\n"

using Bytes = std::vector<u_char>;

struct ip4_addr
{
    u_char b1, b2, b3, b4;

    ip4_addr() = default;
    ip4_addr(u_char c1, u_char c2, u_char c3, u_char c4);
    explicit ip4_addr(const std::string &s);
    ip4_addr(const in_addr &waddr);

    operator in_addr() const;
    operator u_int() const;

    bool operator==(const ip4_addr &other) const;
    bool operator!=(const ip4_addr &other) const;
    u_int operator&(const ip4_addr &other) const;

    bool same_subnet(const ip4_addr &other, const ip4_addr &netmask) const;
    static bool is_valid(const std::string &s);
    static ip4_addr from_hostname(const std::string &name, bool &succ);
};

struct eth_addr
{
    u_char b1, b2, b3, b4, b5, b6;

    bool operator==(const eth_addr &other) const;
    bool operator!=(const eth_addr &other) const;
};

std::ostream &operator<<(std::ostream &out, const eth_addr &addr);
std::ostream &operator<<(std::ostream &out, const ip4_addr &addr);
std::ostream &operator<<(std::ostream &out, const in_addr &addr);
std::ostream &operator<<(std::ostream &out, const in6_addr &addr);
std::ostream &operator<<(std::ostream &out, const sockaddr *addr);

/*
 * Protocol data structure details below as follows
 */

struct _ethernet_header_detail
{
    eth_addr dea;      // Destination address
    eth_addr sea;      // Source address
    u_short eth_type;  // Ethernet type
};

struct ethernet_header
{
    _ethernet_header_detail d;  // Ethernet detail
};

struct _arp_header_detail
{
    u_short hw_type;   // Hardware type
    u_short proto;     // Protocol
    u_char hw_len;     // Hardware address length
    u_char proto_len;  // Protocol address length
    u_short op;        // Operation code
    eth_addr sea;      // Source ethernet address
    ip4_addr sia;      // Source ip address
    eth_addr dea;      // Destination ethernet address
    ip4_addr dia;      // Destination ip address

    bool is_fake() const;
    bool is_typical() const;
};

struct arp_header
{
    ethernet_header h;     // Ethernet header
    _arp_header_detail d;  // ARP detail
};

struct _ip_header_detail
{
    u_char ver_ihl;    // Version (4 bits) + Internet header length (4 bits)
    u_char tos;        // Type of service
    u_short tlen;      // Total length
    u_short id;        // Identification
    u_short flags_fo;  // Flags (3 bits) + Fragment offset (13 bits)
    u_char ttl;        // Time to live
    u_char proto;      // Protocol
    u_short crc;       // Header checksum
    ip4_addr sia;      // Source address
    ip4_addr dia;      // Destination address

    bool almost_same(const _ip_header_detail &other) const;
};

struct ip_header
{
    ethernet_header h;    // Ethernet header
    _ip_header_detail d;  // IP detail
};

struct _icmp_header_detail
{
    u_char type;  // Type
    u_char code;  // Code
    u_short crc;  // Checksum as a whole
    u_short id;   // Identification
    u_short sn;   // Serial number

    bool is_typeof_error() const;
};

struct icmp_header
{
    ip_header h;            // IPv4 header
    _icmp_header_detail d;  // ICMP detail
};

struct _icmp_netmask_detail
{
    ip4_addr mask;  // Subnet address mask
};

struct _icmp_error_detail
{
    icmp_header h_aux;       // Not part of detail, only used for print
    _ip_header_detail e_ip;  // Error ip header
    u_char buf[8];           // At least 8-bit behind ip header
};

struct _pseudo_udp_header_detail
{
    ip4_addr sia;     // Source address
    ip4_addr dia;     // Destination address
    u_char zero_pad;  // Zero
    u_char proto;     // Protocol
    u_short len;      // Datagram length
};

struct _udp_header_detail
{
    u_short sport;  // Source port
    u_short dport;  // Destination port
    u_short len;    // Datagram length, >= 8
    u_short crc;    // Checksum
};

struct udp_header
{
    ip_header h;           // IPv4 header
    _udp_header_detail d;  // UDP detail
};

struct dns_header
{
    u_short id;     // Identification
    u_short flags;  // Flags
    u_short qrn;    // Query number
    u_short rrn;    // Reply resource record number
    u_short arn;    // Auth resource record number
    u_short ern;    // Extra resource record number
};

struct dns_query_tailer
{
    u_short type;  // Query type
    u_short cls;   // Query class
};

struct dns_query_record
{
    std::string domain;  // Query domain
    dns_query_tailer t;  // Query tailer
};

struct dns_res_record
{
    std::string domain;  // Domain
    u_short type;        // Query type
    u_short cls;         // Query class
    u_int ttl;           // Time to live
    u_short data_len;    // Resource data length
    Bytes res_data;      // Resource data
};

struct dns_reply
{
    dns_header h;                         // DNS header
    std::vector<dns_query_record> query;  // Query
    std::vector<dns_res_record> reply;    // Reply
    std::vector<dns_res_record> auth;     // Auth
    std::vector<dns_res_record> extra;    // Extra
};

u_short calc_checksum(const void *data, size_t len_in_byte);

u_short calc_udp_checksum(const void *data, size_t len_in_byte,
                          const _pseudo_udp_header_detail &dt);

u_short calc_udp_checksum(const void *data, size_t len_in_byte, const _ip_header_detail &it);

std::ostream &operator<<(std::ostream &out, const _ethernet_header_detail &detail);
std::ostream &operator<<(std::ostream &out, const ethernet_header &header);
std::ostream &operator<<(std::ostream &out, const _arp_header_detail &detail);
std::ostream &operator<<(std::ostream &out, const arp_header &header);
std::ostream &operator<<(std::ostream &out, const _ip_header_detail &detail);
std::ostream &operator<<(std::ostream &out, const ip_header &header);
std::ostream &operator<<(std::ostream &out, const icmp_header &header);
std::ostream &operator<<(std::ostream &out, const _icmp_netmask_detail &detail);
std::ostream &operator<<(std::ostream &out, const _icmp_error_detail &detail);
std::ostream &operator<<(std::ostream &out, const _udp_header_detail &detail);
std::ostream &operator<<(std::ostream &out, const udp_header &header);
std::ostream &operator<<(std::ostream &out, const dns_header &header);
std::ostream &operator<<(std::ostream &out, const dns_query_record &record);
std::ostream &operator<<(std::ostream &out, const dns_res_record &record);
std::ostream &operator<<(std::ostream &out, const dns_reply &reply);