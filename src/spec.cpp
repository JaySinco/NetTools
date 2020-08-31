#include <ws2tcpip.h>
#include "spec.h"

ip4_addr::ip4_addr(u_char c1, u_char c2, u_char c3, u_char c4)
    :b1(c1), b2(c2), b3(c3), b4(c4) {}

ip4_addr::ip4_addr(const std::string &s)
{
    in_addr waddr;
    if (inet_pton(AF_INET, s.c_str(), &waddr) != 1) {
        throw std::runtime_error(nt::sout << "failed to decode ipv4 address: " << s);
    }
    *this = ip4_addr(waddr);
}

ip4_addr::ip4_addr(const in_addr &waddr)
{
    b1 = waddr.S_un.S_un_b.s_b1;
    b2 = waddr.S_un.S_un_b.s_b2;
    b3 = waddr.S_un.S_un_b.s_b3;
    b4 = waddr.S_un.S_un_b.s_b4;
}

ip4_addr::operator in_addr() const
{
    in_addr waddr;
    waddr.S_un.S_un_b.s_b1 = b1;
    waddr.S_un.S_un_b.s_b2 = b2;
    waddr.S_un.S_un_b.s_b3 = b3;
    waddr.S_un.S_un_b.s_b4 = b4;
    return waddr;
}

ip4_addr::operator u_int() const
{
    auto i = reinterpret_cast<const u_int*>(this);
    return *i;
}

bool ip4_addr::operator==(const ip4_addr &other) const
{
    return b1 == other.b1 && b2 == other.b2 &&
        b3 == other.b3 && b4 == other.b4;
}

bool ip4_addr::operator!=(const ip4_addr &other) const
{  
    return !(*this == other);
}

u_int ip4_addr::operator&(const ip4_addr &other) const
{
    auto i = reinterpret_cast<const u_int*>(this);
    auto j = reinterpret_cast<const u_int*>(&other);
    return ntohl(*i) & ntohl(*j);
}

bool eth_addr::operator==(const eth_addr &other) const
{
    return b1 == other.b1 && b2 == other.b2 && b3 == other.b3 &&
        b4 == other.b4 && b5 == other.b5 && b6 == other.b6;
}

bool eth_addr::operator!=(const eth_addr &other) const
{
    return !(*this == other);
}

bool eth_ip4_arp::fake() const
{
    if (ntohs(op) == ARP_REPLY_OP && sea == dea && sia != dia) {
        return true;
    }
    return false;
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

std::ostream &operator<<(std::ostream &out, const ip4_addr &addr)
{
    return out << static_cast<in_addr>(addr);
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
    out << "\tEthernet Type: IPv4\n";
    out << "\t------------------------\n";
    out << "\tIP Header Size: " << header_size << " bytes\n";
    out << "\tIP Total Size: " << total_size << " bytes\n";
    out << "\tIP Header Checksum: " << calc_checksum(ip4_data, header_size) << "\n";
    out << "\tIP Identification: " << ntohs(ip4_data->id) << "\n";
    out << "\tIP Flags: " << (ntohs(ip4_data->flags_fo) >> 13) << "\n";
    out << "\tIP Fragment Offset: " << (ntohs(ip4_data->flags_fo) & (~0>>3)) << "\n";
    out << "\tTTL: " << static_cast<int>(ip4_data->ttl) << "\n";
    out << "\tSource Ip: " << ip4_data->sia << "\n";
    out << "\tDestination Ip: " << ip4_data->dia << "\n";
    auto ptr = reinterpret_cast<const u_char *>(ip4_data) + sizeof(ip4_header);
    switch (ip4_data->proto) {
    case IPv4_TYPE_ICMP:
    {
        out << "\tIP Type: ICMP\n";
        auto ih = reinterpret_cast<const icmp_header*>(ptr);
        print_icmp(out, ih, total_size - header_size);
        break;
    }
    case IPv4_TYPE_TCP:
        out << "\tIP Type: TCP\n";
        break;
    case IPv4_TYPE_UDP:
        out << "\tIP Type: UDP\n";
        break;
    default:
        out << "\tIP Type: " << ip4_data->proto << "\n";
        break;
    }
    return out;
}

std::ostream &print_icmp(std::ostream &out, const icmp_header *icmp_data, size_t length)
{
    std::ostringstream desc, body;
    switch (icmp_data->type) {
    case 3: case 4: case 5: case 11: case 12:
        desc << "error";
        break;
    case ICMP_TYPE_PING_ASK:
    case ICMP_TYPE_PING_REPLY: {
        if (icmp_data->type == ICMP_TYPE_PING_ASK) desc << "ping-ask";
        if (icmp_data->type == ICMP_TYPE_PING_REPLY) desc << "ping-reply";
        auto mh = reinterpret_cast<const icmp_ping*>(icmp_data);
        body << "\tICMP Checksum: " << calc_checksum(mh, sizeof(icmp_ping)) << "\n";
        body << "\tICMP Identification: " << ntohs(mh->id) << "\n";
        body << "\tICMP Serial No.: " << ntohs(mh->sn) << "\n";
        break;
    }
    case 9: case 10:
        desc << "router";
        break;
    case 13: case 14:
        desc << "timestamp";
        break;
    case ICMP_TYPE_NETMASK_ASK:
    case ICMP_TYPE_NETMASK_REPLY: {
        if (icmp_data->type == ICMP_TYPE_NETMASK_ASK) desc << "netmask-ask";
        if (icmp_data->type == ICMP_TYPE_NETMASK_REPLY) desc << "netmask-reply";
        auto mh = reinterpret_cast<const icmp_addr_mask*>(icmp_data);
        body << "\tICMP Checksum: " << calc_checksum(mh, sizeof(icmp_addr_mask)) << "\n";
        body << "\tICMP Identification: " << ntohs(mh->id) << "\n";
        body << "\tICMP Serial No.: " << ntohs(mh->sn) << "\n";
        body << "\tICMP Netmask: " << mh->mask << "\n";
        break;
    }
    }
    out << "\t------------------------\n";
    out << "\tICMP Type: " << desc.str() << "(" << int(icmp_data->type) << ")" << "\n";
    out << "\tICMP Size: " << length << " bytes\n";
    out << "\tICMP Code: " << int(icmp_data->code) << "\n";
    out << body.str();
    return out;
}