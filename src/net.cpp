#include <thread>
#include <atomic>
#include "net.h"

bool ip2mac(
    pcap_t *adhandle,
    const adapter_info &apt_info,
    const ip4_addr &ip,
    eth_addr &mac,
    int timeout_ms)
{
    auto start_tm = std::chrono::system_clock::now();
    std::atomic<bool> over = false;
    std::thread send_loop_t([&]{
        while (!over) {
            send_arp(adhandle, ARP_REQUEST_OP, apt_info.mac, apt_info.ip, PLACEHOLDER_ETH_ADDR, ip);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    });
    
    int res;
    pcap_pkthdr *header;
    const u_char *pkt_data;
    bool succ = false;
    while((res = pcap_next_ex(adhandle, &header, &pkt_data)) >= 0)
    {
        if (timeout_ms > 0)
        {
            auto now_tm = std::chrono::system_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now_tm - start_tm);
            if (duration.count() >= timeout_ms) {
                LOG(ERROR) << "haven't got arp reply after " << duration.count() << " ms";
                break;
            }

        }
        if (res == 0) {
            continue;
        }
        auto eh = reinterpret_cast<const ethernet_header*>(pkt_data);
        if (ntohs(eh->eth_type) == ETHERNET_TYPE_ARP) {
            auto ah = reinterpret_cast<const eth_ip4_arp*>(pkt_data + sizeof(ethernet_header));
            if (ntohs(ah->op) == ARP_REPLY_OP) {
                if (!ah->fake() && ah->sia == ip) {
                    mac = ah->sea;
                    succ = true;
                    break;
                }
            }
        }
    }
    if (res == -1) {
        LOG(ERROR) << "failed to read packets: " << pcap_geterr(adhandle);
    }
    over = true;
    send_loop_t.join();
    return succ;
}

bool send_arp(
    pcap_t *adhandle,
    u_short op,
    const eth_addr &sea,
    const ip4_addr &sia,
    const eth_addr &dea,
    const ip4_addr &dia)
{
    u_char packet[sizeof(ethernet_header) + sizeof(eth_ip4_arp)] = { 0 };
    auto eh = reinterpret_cast<ethernet_header*>(packet);
    eh->dea = BROADCAST_ETH_ADDR;
    eh->sea = sea;
    eh->eth_type = htons(ETHERNET_TYPE_ARP);
    auto ah = reinterpret_cast<eth_ip4_arp*>(packet + sizeof(ethernet_header));
    ah->hw_type = htons(ARP_HARDWARE_TYPE_ETHERNET);
    ah->proto = htons(ETHERNET_TYPE_IPv4);
    ah->hw_len = ETHERNET_ADDRESS_LEN;
    ah->proto_len = IPV4_ADDRESS_LEN;
    ah->op = htons(op);
    ah->sea = sea;
    ah->sia = sia;
    ah->dea = dea;
    ah->dia = dia;
    if (pcap_sendpacket(adhandle, packet, sizeof(packet)/sizeof(packet[0])) != 0)
    {
        LOG(ERROR) << "failed to send packet: " << pcap_geterr(adhandle);
        return false;
    }
    return true;
}

std::ostream &operator<<(std::ostream &out, const pcap_if_t *dev)
{
    out << dev->name << std::endl;
    bool get_detail = false;
    adapter_info apt_info;
    for (pcap_addr_t *a = dev->addresses; a; a = a->next) {
        if (a->addr && a->addr->sa_family == AF_INET) {
            auto waddr = reinterpret_cast<const sockaddr_in*>(a->addr)->sin_addr;
            apt_info = adapter_info(waddr, true);
            if (apt_info.ip == waddr) {
                get_detail = true;
                out << "\tDescription: " << apt_info.desc << std::endl;
                out << "\tMAC: " << apt_info.mac << std::endl;
            }
            break;
        }
    }
    if (!get_detail) {
        out << "\tDescription: " << dev->description << std::endl;
    }
    out << "\tLoopback: " << ((dev->flags & PCAP_IF_LOOPBACK) ? "yes" : "no") << std::endl;
    for (pcap_addr_t *a = dev->addresses; a; a = a->next) {
        out << "\t---------------------------------------" << std::endl;
        if (a->addr) {
            out << "\tAddress: " << a->addr << std::endl;
            auto waddr = reinterpret_cast<const sockaddr_in*>(a->addr)->sin_addr;
            if (get_detail && apt_info.ip == waddr &&
                apt_info.gateway != PLACEHOLDER_IPv4_ADDR)
            {
                out << "\tGateway: " << apt_info.gateway << std::endl;
            }
        }
        if (a->netmask) out << "\tNetmask: " << a->netmask << std::endl;
        if (a->broadaddr) out << "\tBroadcast: " << a->broadaddr << std::endl;
        if (a->dstaddr) out << "\tDestination: " << a->dstaddr << std::endl;
    }
    return out;
}

pcap_t *open_target_adaptor(const ip4_addr &ip, bool exact_match, adapter_info &apt_info)
{
    apt_info = adapter_info(ip, exact_match);
    if (apt_info.name.size() == 0) {
        throw std::runtime_error(nt::sout << "no adapter found in same local network with " << ip);
    }
    pcap_t *adhandle;
    char errbuf[PCAP_ERRBUF_SIZE];
    if (!(adhandle= pcap_open(apt_info.name.c_str(), 65536, PCAP_OPENFLAG_PROMISCUOUS, 1000, NULL, errbuf)))
    {
        throw std::runtime_error(nt::sout << "failed to open the adapter: " << apt_info.name);
    }
    return adhandle;
}

u_short calc_checksum(const void *data, size_t len_in_byte)
{
    if (len_in_byte % 2 != 0) {
        throw std::runtime_error("calculate checksum: data length is not an integer of 2");
    }
    u_long checksum = 0;
    auto check_ptr = reinterpret_cast<const u_short *>(data);
    for (int i = 0; i < len_in_byte / 2; ++i) {
        checksum += check_ptr[i];
        checksum = (checksum >> 16) + (checksum & 0xffff);
    }
    return static_cast<u_short>(~checksum);
}

std::ostream &print_packet(
    std::ostream &out,
    const pcap_pkthdr *header,
    const u_char *pkt_data)
{
    time_t local_tv_sec = header->ts.tv_sec;
    tm ltime;
    localtime_s(&ltime, &local_tv_sec);
    char timestr[16];
    strftime(timestr, sizeof(timestr), "%H:%M:%S", &ltime);
    out << timestr << "." << std::setw(6) << std::left << header->ts.tv_usec << " ";
    auto eh = reinterpret_cast<const ethernet_header*>(pkt_data);
    out << eh->sea << " > " << eh->dea << std::endl;
    u_short ethtyp = ntohs(eh->eth_type);
    const u_char *ptr = pkt_data + sizeof(ethernet_header);
    switch (ethtyp)
    {
    case ETHERNET_TYPE_IPv4:
    {
        auto ih = reinterpret_cast<const ip4_header*>(ptr);
        out << ih;
        break;
    }
    case ETHERNET_TYPE_IPv6:
        out << "\tEthernet type: IPv6\n";
        break;
    case ETHERNET_TYPE_ARP:
    case ETHERNET_TYPE_RARP:
    {
        auto ah = reinterpret_cast<const eth_ip4_arp*>(ptr);
        out << ah;
        break;
    }
    default:
        LOG(ERROR) << "unknow ethernet type: 0x" << std::hex << ethtyp << std::dec;
    }
    return out;
}

std::ostream &operator<<(std::ostream &out, const eth_ip4_arp *arp_data)
{
    if (ntohs(arp_data->hw_type) != ARP_HARDWARE_TYPE_ETHERNET ||
        ntohs(arp_data->proto) != ETHERNET_TYPE_IPv4 ||
        arp_data->hw_len != ETHERNET_ADDRESS_LEN ||
        arp_data->proto_len != IPV4_ADDRESS_LEN)
    {
        LOG(ERROR) << "not typical ethernet-ipv4 arp/rarp";
        return out;
    }
    switch (ntohs(arp_data->op)) {
    case ARP_REQUEST_OP:
        out << "\tEthernet type: ARP Requset" << (arp_data->fake() ? "*" : "") << "\n";
        out << "\tDescription: " << arp_data->sia << " asks: who has " <<  arp_data->dia << "?\n";
        break;
    case ARP_REPLY_OP:
        out << "\tEthernet type: ARP Reply" << (arp_data->fake() ? "*" : "") << "\n";
        out << "\tDescription: " <<  arp_data->sia << " tells " << arp_data->dia << ": i am at " << arp_data->sea << ".\n";
        break;
    case RARP_REQUEST_OP:
        out << "\tEthernet type: RARP Requset\n";
        break;
    case RARP_REPLY_op:
        out << "\tEthernet type: RARP Reply\n";
        break;
    }
    out << "\tSource Mac: " << arp_data->sea << "\n";
    out << "\tSource Ip: " << arp_data->sia << "\n";
    out << "\tDestination Mac: " << arp_data->dea << "\n";
    out << "\tDestination Ip: " << arp_data->dia << "\n";
    return out;
}

std::ostream &operator<<(std::ostream &out, const ip4_header *ip4_data)
{
    if ((ip4_data->ver_ihl >> 4) != 4) {
        LOG(ERROR) << "ip protocol version is not 4";
        return out;
    }
    size_t header_size = 4 * (ip4_data->ver_ihl & 0xf);
    size_t total_size = ntohs(ip4_data->tlen);
    out << "\tEthernet type: IPv4\n";
    out << "\tIPv4 Header Size: " << header_size << " bytes\n";
    out << "\tIPv4 Total Size: " << total_size << " bytes\n";
    out << "\tIPv4 Header Checksum: " << calc_checksum(ip4_data, header_size) << "\n";
    out << "\tTTL: " << static_cast<int>(ip4_data->ttl) << "\n";
    out << "\tSource Ip: " << ip4_data->sia << "\n";
    out << "\tDestination Ip: " << ip4_data->dia << "\n";
    switch (ip4_data->proto) {
    case IPv4_TYPE_ICMP:
        out << "\tIPv4 type: ICMP\n";
        break;
    case IPv4_TYPE_TCP:
        out << "\tIPv4 type: TCP\n";
        break;
    case IPv4_TYPE_UDP:
        out << "\tIPv4 type: UDP\n";
        break;
    default:
        out << "\tIPv4 type: " << ip4_data->proto << "\n";
        break;
    }
    return out;
}