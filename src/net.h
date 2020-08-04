#pragma once
#include "type.h"

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

std::ostream &operator<<(std::ostream &out, const eth_ip4_arp *arp_data);
std::ostream &operator<<(std::ostream &out, const pcap_if_t *dev);