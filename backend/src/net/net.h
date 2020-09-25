#pragma once
#include <chrono>
#include <functional>
#include "pcap.h"
#include "spec.h"
#define NT_TRY try {
#define NT_CATCH \
    }            \
    catch (const std::runtime_error &e) { LOG(ERROR) << e.what(); }

using Bytes = std::vector<u_char>;

struct adapter_info
{
    std::string name;
    std::string desc;
    ip4_addr ip;
    ip4_addr mask;
    ip4_addr gateway;
    eth_addr mac;

    adapter_info() = default;

    static adapter_info select_ip(const ip4_addr &subnet_ip, bool exact_match);
    static adapter_info select_auto();
};

struct wsa_guard
{
    wsa_guard();
    ~wsa_guard();
};

struct ip_mac_cache
{
    ip4_addr ip;
    eth_addr mac;
    std::chrono::time_point<std::chrono::system_clock> tm;
};

std::ostream &operator<<(std::ostream &out, const adapter_info &apt);
std::ostream &operator<<(std::ostream &out, const pcap_if_t *dev);

u_short rand_ushort();
void get_time_of_day(timeval *tv);
long operator-(const timeval &tv1, const timeval &tv2);

Bytes encode_domain_name(const std::string &domain);
Bytes make_dns_query(const std::string &domain, u_short &id);

pcap_t *open_target_adaptor(const ip4_addr &ip, bool exact_match, adapter_info &apt_info);
std::ostream &print_packet(std::ostream &out, const pcap_pkthdr *pkthdr, const ethernet_header *eh);

using packet_loop_callback = std::function<int(pcap_pkthdr *, const ethernet_header *)>;
int packet_loop(pcap_t *adhandle, const std::chrono::system_clock::time_point &start_tm,
                int timeout_ms, const packet_loop_callback &cb);

int ip2mac(pcap_t *adhandle, const adapter_info &apt_info, const ip4_addr &ip, eth_addr &mac,
           int timeout_ms, bool use_cache = true);

int send_arp(pcap_t *adhandle, u_short op, const eth_addr &sea, const ip4_addr &sia,
             const eth_addr &dea, const ip4_addr &dia);

int send_ip(pcap_t *adhandle, const eth_addr &dea, const eth_addr &sea, u_char proto,
            const ip4_addr &sia, const ip4_addr &dia, u_char ttl, void *ip_data, size_t len_in_byte,
            ip_header &ih_send);

int ping(pcap_t *adhandle, const adapter_info &apt_info, const ip4_addr &target_ip, u_char ttl,
         int timeout_ms, long &cost_ms, ip_header &ih_recv, _icmp_error_detail &d_err);
