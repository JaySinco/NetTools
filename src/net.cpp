#include <winsock2.h>
#include <iphlpapi.h>
#include "net.h"

bool get_adapter_info_by_ip4(const ip4_addr &ip, adapter_info &info)
{
    u_long buflen = sizeof(IP_ADAPTER_INFO);
    auto pAdapterInfo = reinterpret_cast<IP_ADAPTER_INFO*>(malloc(sizeof(IP_ADAPTER_INFO)));
    if (GetAdaptersInfo(pAdapterInfo, &buflen) == ERROR_BUFFER_OVERFLOW) {
        free(pAdapterInfo);
        pAdapterInfo = reinterpret_cast<IP_ADAPTER_INFO*>(malloc(buflen));
        if (GetAdaptersInfo(pAdapterInfo, &buflen) != NO_ERROR) {
            LOG(ERROR) << "failed to call GetAdaptersInfo";
            free(pAdapterInfo);
            return false;
        }
    }
    PIP_ADAPTER_INFO pAdapter = NULL;
    pAdapter = pAdapterInfo;
    bool found = false;
    while (pAdapter) {
        ip4_addr addr;
        ip4_from_string(pAdapter->IpAddressList.IpAddress.String, addr);
        if (addr == ip) {
            found = true;
            info.name = pAdapter->AdapterName;
            info.desc = pAdapter->Description;
            info.ip = ip;
            ip4_from_string(pAdapter->IpAddressList.IpMask.String, info.mask);
            ip4_from_string(pAdapter->GatewayList.IpAddress.String, info.gateway);
            if (pAdapter->AddressLength != sizeof(eth_addr)) {
                LOG(WARNING) << "wrong address length: " << pAdapter->AddressLength;
            }
            else {
                auto c = reinterpret_cast<u_char*>(&info.mac);
                for (unsigned i = 0; i < pAdapter->AddressLength; ++i) {
                    c[i] = pAdapter->Address[i];
                }
            }
            break;
        }
        pAdapter = pAdapter->Next;
    }
    free(pAdapterInfo);
    return found;
}

bool ip4_from_string(const std::string &s, ip4_addr &addr)
{
    in_addr waddr;
    if (inet_pton(AF_INET, s.c_str(), &waddr) != 1) {
        LOG(ERROR) << "failed to decode ipv4 address: " << s;
        return false;
    }
    addr = ip4_from_win(waddr);
    return true;
}

in_addr ip4_to_win(const ip4_addr &addr)
{
    in_addr waddr;
    waddr.S_un.S_un_b.s_b1 = addr.b1;
    waddr.S_un.S_un_b.s_b2 = addr.b2;
    waddr.S_un.S_un_b.s_b3 = addr.b3;
    waddr.S_un.S_un_b.s_b4 = addr.b4;
    return waddr;
}

ip4_addr ip4_from_win(const in_addr &waddr)
{
    ip4_addr addr;
    addr.b1 = waddr.S_un.S_un_b.s_b1;
    addr.b2 = waddr.S_un.S_un_b.s_b2;
    addr.b3 = waddr.S_un.S_un_b.s_b3;
    addr.b4 = waddr.S_un.S_un_b.s_b4;
    return addr;
}

u_int operator&(const ip4_addr &a, const ip4_addr &b)
{
    auto i = reinterpret_cast<const u_int*>(&a);
    auto j = reinterpret_cast<const u_int*>(&b);
    return ntohl(*i) & ntohl(*j);
}

bool operator==(const ip4_addr &a, const ip4_addr &b)
{
    return a.b1 == b.b1 && a.b2 == b.b2 &&
        a.b3 == b.b3 && a.b4 == b.b4;
}

std::ostream &operator<<(std::ostream &out, const in_addr &addr)
{
    u_long ip4 = addr.s_addr;
    auto c = reinterpret_cast<const u_char*>(&ip4);
    out << int(c[0]);
    for (int i = 1; i < 4; ++i) {
        out << "." << int(c[i]);
    }
    return out;
}

std::ostream &operator<<(std::ostream &out, const in6_addr &addr)
{
    const u_short *h = addr.u.Word;
    out << std::hex << ntohs(h[0]);
    for (int i = 1; i < 8; ++i) {
        out << ":" << ntohs(h[i]);
    }
    return out << std::dec;
}

std::ostream &operator<<(std::ostream &out, const sockaddr *addr)
{
    if (addr->sa_family == AF_INET) {
        out << reinterpret_cast<const sockaddr_in*>(addr)->sin_addr;
    }
    else if (addr->sa_family == AF_INET6) {
        out << reinterpret_cast<const sockaddr_in6*>(addr)->sin6_addr;
    }
    else if (addr->sa_family == AF_UNSPEC) {
        out << "(unspecified)";
    }
    else {
        LOG(ERROR) << "unknow ip address family: " << addr->sa_family;
    }
    return out;
}

std::ostream &operator<<(std::ostream &out, const pcap_if_t *dev)
{
    out << dev->name << std::endl;
    bool get_detail = false;
    for (pcap_addr_t *a = dev->addresses; a; a = a->next) {
        if (a->addr && a->addr->sa_family == AF_INET) {
            auto waddr = reinterpret_cast<const sockaddr_in*>(a->addr)->sin_addr;
            adapter_info apt_info;
            if (get_adapter_info_by_ip4(ip4_from_win(waddr), apt_info)) {
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
        if (a->addr) out << "\tAddress: " << a->addr << std::endl;
        if (a->netmask) out << "\tNetmask: " << a->netmask << std::endl;
        if (a->broadaddr) out << "\tBroadcast: " << a->broadaddr << std::endl;
        if (a->dstaddr) out << "\tDestination: " << a->dstaddr << std::endl;
    }
    return out;
}

std::ostream &operator<<(std::ostream &out, const ip4_addr &addr)
{
    return out << ip4_to_win(addr);
}

std::ostream &operator<<(std::ostream &out, const eth_addr &addr)
{
    auto c = reinterpret_cast<const u_char*>(&addr);
    out << std::hex << int(c[0]);
    for (int i = 1; i < 6; ++i) {
        out << "-" << int(c[i]);
    }
    return out << std::dec;
}

std::ostream &operator<<(std::ostream &out, const eth_ip4_arp *arp_data)
{
    if (ntohs(arp_data->hw_type) != ARP_HARDWARE_TYPE_ETHERNET ||
        ntohs(arp_data->proto) != ARP_HARDWARE_PROTO_IP ||
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
        out << "\tDescription: " <<  arp_data->sia << " replies to " << arp_data->dia << ": i am at " << arp_data->sea << ".\n";
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

std::ostream &print_packet(std::ostream &out, const pcap_pkthdr *header, const u_char *pkt_data)
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