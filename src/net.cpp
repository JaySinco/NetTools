#include "net.h"

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
    switch (ethtyp)
    {
    case ETHERNET_TYPE_IPv4:
        out << "\tEthernet type: IPv4\n";
        break;
    case ETHERNET_TYPE_IPv6:
        out << "\tEthernet type: IPv6\n";
        break;
    case ETHERNET_TYPE_ARP:
    case ETHERNET_TYPE_RARP:
    {
        auto ah = reinterpret_cast<const eth_ip4_arp*>(pkt_data + sizeof(ethernet_header));
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
        out << "\tEthernet type: ARP\n";
        out << "\tDescription: " << arp_data->sia << " asks: who has " <<  arp_data->dia << "?\n";
        out << "\tOperation: Requset\n";
        break;
    case ARP_REPLY_OP:
        out << "\tEthernet type: ARP\n";
        out << "\tDescription: " <<  arp_data->sia << " tells " << arp_data->dia << ": i am at " << arp_data->sea << ".\n";
        out << "\tOperation: Reply\n";
        break;
    case RARP_REQUEST_OP:
        out << "\tEthernet type: RARP\n";
        out << "\tOperation: Requset\n";
        break;
    case RARP_REPLY_op:
        out << "\tEthernet type: RARP\n";
        out << "\tOperation: Reply\n";
        break;
    }
    out << "\tSource Mac: " << arp_data->sea << "\n";
    out << "\tSource Ip: " << arp_data->sia << "\n";
    out << "\tDestination Mac: " << arp_data->dea << "\n";
    out << "\tDestination Ip: " << arp_data->dia << "\n";
    return out;
}