#pragma once
#include "type.h"
#define NT_TRY try {
#define NT_CATCH } catch (const std::runtime_error &e) { LOG(ERROR) << e.what(); }

bool ip2mac(
    pcap_t *adhandle,
    const adapter_info &apt_info,
    const ip4_addr &ip,
    eth_addr &mac,
    int timeout_ms);

bool send_arp(
    pcap_t *adhandle,
    u_short op,
    const eth_addr &sea,
    const ip4_addr &sia,
    const eth_addr &dea,
    const ip4_addr &dia);

std::ostream &print_packet(
    std::ostream &out,
    const pcap_pkthdr *header,
    const u_char *pkt_data);

u_short calc_checksum(const void *data, size_t len_in_byte);
pcap_t *open_target_adaptor(const ip4_addr &ip, bool exact_match, adapter_info &apt_info);
std::ostream &operator<<(std::ostream &out, const ip4_header *ip4_data);
std::ostream &operator<<(std::ostream &out, const eth_ip4_arp *arp_data);
std::ostream &operator<<(std::ostream &out, const pcap_if_t *dev);