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

bool arp_header::is_fake() const
{
    if (ntohs(op) == ARP_REPLY_OP && sea == dea && sia != dia) {
        return true;
    }
    return false;
}

bool arp_header::is_typical() const
{
    return ntohs(hw_type) == ARP_HARDWARE_TYPE_ETHERNET && ntohs(proto) == ETHERNET_TYPE_IPv4 &&
        hw_len == ETHERNET_ADDRESS_LEN && proto_len == IPV4_ADDRESS_LEN;
}

std::ostream &operator<<(std::ostream &out, const ethernet_header &data)
{
    std::ostringstream ss;
    switch (ntohs(data.eth_type))
    {
    case ETHERNET_TYPE_IPv4: ss << "IPv4"; break;
    case ETHERNET_TYPE_IPv6: ss << "IPv6"; break;
    case ETHERNET_TYPE_ARP : ss << "ARP" ; break;
    case ETHERNET_TYPE_RARP: ss << "RARP"; break;
    default:
        ss << "Unknow(" << ntohs(data.eth_type) << ")";
        break;
    }
    out << "\tEthernet Type: " << ss.str() << std::endl;
    out << "\tSource Mac: " << data.sea << std::endl;
    out << "\tDestination Mac: " << data.dea << std::endl;
    return out;
}

std::ostream &operator<<(std::ostream &out, const arp_header &data)
{
    out << data.h_eth << DELIMITER_LINE;

    if (!data.is_typical()) {
        out << "\tDescription: Not typical ethernet-ipv4 arp/rarp" << std::endl;
        return out;
    }
    switch (ntohs(data.op))
    {
    case ARP_REQUEST_OP:
        out << "\tARP Type: Requset" << (data.is_fake() ? "*" : "") << "\n";
        out << "\tDescription: " << data.sia << " asks: who has " <<  data.dia << "?\n";
        break;
    case ARP_REPLY_OP:
        out << "\tARP Type: Reply" << (data.is_fake() ? "*" : "") << "\n";
        out << "\tDescription: " <<  data.sia << " tells " << data.dia << ": i am at " << data.sea << ".\n";
        break;
    case RARP_REQUEST_OP:
        out << "\tRARP Type: Requset\n";
        break;
    case RARP_REPLY_op:
        out << "\tRARP Type: Reply\n";
        break;
    }
    out << "\tSource Mac: " << data.sea << "\n";
    out << "\tSource Ip: " << data.sia << "\n";
    out << "\tDestination Mac: " << data.dea << "\n";
    out << "\tDestination Ip: " << data.dia << "\n";
    return out;
}

std::ostream &operator<<(std::ostream &out, const ip_header &data)
{
    out << data.h_eth << DELIMITER_LINE;

    if ((data.ver_ihl >> 4) != 4) {
        out << "\tDescription: IP protocol version is not 4" << std::endl;
        return out;
    }
    size_t header_size = 4 * (data.ver_ihl & 0xf);
    size_t total_size = ntohs(data.tlen);
    out << "\tIP Header Size: " << header_size << " bytes\n";
    out << "\tIP Total Size: " << total_size << " bytes\n";
    out << "\tIP Header Checksum: " << calc_checksum(IP_HEADER_START(&data), header_size) << "\n";
    out << "\tIP Identification: " << ntohs(data.id) << "\n";
    out << "\tIP Flags: " << (ntohs(data.flags_fo) >> 13) << "\n";
    out << "\tIP Fragment Offset: " << (ntohs(data.flags_fo) & (~0>>3)) << "\n";
    out << "\tTTL: " << static_cast<int>(data.ttl) << "\n";
    out << "\tSource Ip: " << data.sia << "\n";
    out << "\tDestination Ip: " << data.dia << "\n";
    std::ostringstream ss;
    switch (data.proto)
    {
    case IPv4_TYPE_ICMP: ss << "ICMP"; break;
    case IPv4_TYPE_TCP : ss << "TCP" ; break;
    case IPv4_TYPE_UDP : ss << "UDP" ; break;
    default:
        ss << "Unknow(" << data.proto << ")";
        break;
    }
    out << "\tIP Type: " << ss.str() << std::endl;
    return out;
}

std::ostream &operator<<(std::ostream &out, const icmp_header &data)
{
    out << data.h_ip << DELIMITER_LINE;

    size_t icmp_len = ntohs(data.h_ip.tlen) - IP_HEADER_SIZE;
    std::ostringstream desc, body;
    switch (data.type)
    {
    case 3:
    case 4:
    case 5:
    case 11:
    case 12: desc << "Error"; break;
    case 9:
    case 10: desc << "Router"; break;
    case 13:
    case 14: desc << "Timestamp"; break;
    case ICMP_TYPE_PING_ASK:
    case ICMP_TYPE_PING_REPLY: {
        if (data.type == ICMP_TYPE_PING_ASK)
            desc << "Ping-ask";
        if (data.type == ICMP_TYPE_PING_REPLY)
            desc << "Ping-reply";
        if (icmp_len > ICMP_HEADER_SIZE) {
            const char *c = reinterpret_cast<const char*>(&data) + sizeof(icmp_header);
            body << "\tPing Echo: " << std::string(c, icmp_len - ICMP_HEADER_SIZE) << "\n";
        }
        break;
    }
    case ICMP_TYPE_NETMASK_ASK:
    case ICMP_TYPE_NETMASK_REPLY: {
        if (data.type == ICMP_TYPE_NETMASK_ASK)
            desc << "Netmask-ask";
        if (data.type == ICMP_TYPE_NETMASK_REPLY)
            desc << "Netmask-reply";
        auto mh = reinterpret_cast<const icmp_netmask_header*>(&data);
        body << "\tICMP Netmask: " << mh->mask << "\n";
        break;
    }
    }
    out << "\tICMP Type: " << desc.str() << "(" << int(data.type) << ")" << "\n";
    out << "\tICMP Size: " << icmp_len << " bytes\n";
    out << "\tICMP Code: " << int(data.code) << "\n";
    body << "\tICMP Checksum: " << calc_checksum(ICMP_HEADER_START(&data), icmp_len) << "\n";
    body << "\tICMP Identification: " << ntohs(data.id) << "\n";
    body << "\tICMP Serial No.: " << ntohs(data.sn) << "\n";
    out << body.str();
    return out;
}