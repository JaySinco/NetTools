#pragma once
#include "pcap.h"
#include "spec.h"
#define NT_TRY try {
#define NT_CATCH } catch (const std::runtime_error &e) { LOG(ERROR) << e.what(); }

struct adapter_info {
    std::string name;
    std::string desc;
    ip4_addr ip;
    ip4_addr mask;
    ip4_addr gateway;
    eth_addr mac;

    adapter_info() = default;
    adapter_info(const ip4_addr &subnet_ip, bool exact_match);
};

u_short rand_u_short();

pcap_t *open_target_adaptor(const ip4_addr &ip, bool exact_match, adapter_info &apt_info);

bool ip2mac(pcap_t *adhandle, const adapter_info &apt_info, const ip4_addr &ip, eth_addr &mac, int timeout_ms);

bool query_mask(pcap_t *adhandle, const adapter_info &apt_info, ip4_addr &netmask, int timeout_ms);

bool send_arp(pcap_t *adhandle, u_short op, const eth_addr &sea, const ip4_addr &sia,
    const eth_addr &dea, const ip4_addr &dia);

bool send_ip4(pcap_t *adhandle, const eth_addr &dea, const eth_addr &sea,
    u_char proto, const ip4_addr &sia, const ip4_addr &dia, void *ip_data, size_t len_in_byte);

std::ostream &print_packet(std::ostream &out, const pcap_pkthdr *header, const u_char *pkt_data);
std::ostream &operator<<(std::ostream &out, const adapter_info &apt);
std::ostream &operator<<(std::ostream &out, const pcap_if_t *dev);