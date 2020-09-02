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

bool ip4_addr::same_subnet(const ip4_addr &other, const ip4_addr &netmask) const
{
    return (*this & netmask) == (other & netmask);
}

bool ip4_addr::is_valid(const std::string &s)
{
    in_addr waddr;
    return (inet_pton(AF_INET, s.c_str(), &waddr) == 1);
}

ip4_addr ip4_addr::from_hostname(const std::string &name, bool &succ)
{
    ip4_addr output_ip = PLACEHOLDER_IPv4_ADDR;
    addrinfo hints = {0};
    hints.ai_family = AF_INET;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = 0;
    hints.ai_socktype = SOCK_STREAM;
    addrinfo *first_addr;
    auto ret = GetAddrInfoA(name.c_str(), nullptr, &hints, &first_addr);
    if (ret == 0 && first_addr != nullptr) {
        succ = true;
        output_ip = reinterpret_cast<sockaddr_in *>(first_addr->ai_addr)->sin_addr;
    }
    else {
        succ = false;
        VLOG(1) << "failed to get ipv4 from host name " << name << ": " << ret;
    }
    return output_ip;
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
    if (ntohs(d.op) == ARP_REPLY_OP && d.sea == d.dea && d.sia != d.dia) {
        return true;
    }
    return false;
}

bool arp_header::is_typical() const
{
    return ntohs(d.hw_type) == ARP_HARDWARE_TYPE_ETHERNET && ntohs(d.proto) == ETHERNET_TYPE_IPv4 &&
        d.hw_len == ETHERNET_ADDRESS_LEN && d.proto_len == IPV4_ADDRESS_LEN;
}

std::ostream &operator<<(std::ostream &out, const ethernet_header &data)
{
    std::ostringstream ss;
    switch (ntohs(data.d.eth_type))
    {
    case ETHERNET_TYPE_IPv4: ss << "IPv4"; break;
    case ETHERNET_TYPE_IPv6: ss << "IPv6"; break;
    case ETHERNET_TYPE_ARP : ss << "ARP" ; break;
    case ETHERNET_TYPE_RARP: ss << "RARP"; break;
    default:
        ss << "Unknow(0x" << std::hex << ntohs(data.d.eth_type) << std::dec << ")";
        break;
    }
    out << "\tEthernet Type: " << ss.str() << std::endl;
    out << "\tSource Mac: " << data.d.sea << std::endl;
    out << "\tDestination Mac: " << data.d.dea << std::endl;
    return out;
}

std::ostream &operator<<(std::ostream &out, const arp_header &data)
{
    out << data.h << DELIMITER_LINE;

    if (!data.is_typical()) {
        out << "\tDescription: Not typical ethernet-ipv4 arp/rarp" << std::endl;
        return out;
    }
    switch (ntohs(data.d.op))
    {
    case ARP_REQUEST_OP:
        out << "\tARP Type: Requset" << (data.is_fake() ? "*" : "") << "\n";
        out << "\tDescription: " << data.d.sia << " asks: who has " <<  data.d.dia << "?\n";
        break;
    case ARP_REPLY_OP:
        out << "\tARP Type: Reply" << (data.is_fake() ? "*" : "") << "\n";
        out << "\tDescription: " <<  data.d.sia << " tells " << data.d.dia << ": i am at " << data.d.sea << ".\n";
        break;
    case RARP_REQUEST_OP:
        out << "\tRARP Type: Requset\n";
        break;
    case RARP_REPLY_op:
        out << "\tRARP Type: Reply\n";
        break;
    }
    out << "\tSource Mac: " << data.d.sea << "\n";
    out << "\tSource Ip: " << data.d.sia << "\n";
    out << "\tDestination Mac: " << data.d.dea << "\n";
    out << "\tDestination Ip: " << data.d.dia << "\n";
    return out;
}

std::ostream &operator<<(std::ostream &out, const ip_header &data)
{
    out << data.h << DELIMITER_LINE;

    if ((data.d.ver_ihl >> 4) != 4) {
        out << "\tDescription: IP protocol version is not 4" << std::endl;
        return out;
    }
    size_t header_size = 4 * (data.d.ver_ihl & 0xf);
    size_t total_size = ntohs(data.d.tlen);
    out << "\tIP Header Size: " << header_size << " bytes\n";
    out << "\tIP Total Size: " << total_size << " bytes\n";
    u_short checksum = calc_checksum(&data.d, header_size);
    out << "\tIP Header Checksum: " << checksum << (checksum == 0 ? "" : "*") << "\n";
    out << "\tIP Identification: " << ntohs(data.d.id) << "\n";
    out << "\tIP Flags: " << (ntohs(data.d.flags_fo) >> 13) << "\n";
    out << "\tIP Fragment Offset: " << (ntohs(data.d.flags_fo) & (~0>>3)) << "\n";
    out << "\tTTL: " << static_cast<int>(data.d.ttl) << "\n";
    out << "\tSource Ip: " << data.d.sia << "\n";
    out << "\tDestination Ip: " << data.d.dia << "\n";
    std::ostringstream ss;
    switch (data.d.proto)
    {
    case IPv4_TYPE_ICMP: ss << "ICMP"; break;
    case IPv4_TYPE_TCP : ss << "TCP" ; break;
    case IPv4_TYPE_UDP : ss << "UDP" ; break;
    default:
        ss << "Unknow(" << data.d.proto << ")";
        break;
    }
    out << "\tIP Type: " << ss.str() << std::endl;
    return out;
}

std::ostream &operator<<(std::ostream &out, const icmp_header &data)
{
    out << data.h << DELIMITER_LINE;

    size_t icmp_len = ntohs(data.h.d.tlen) - sizeof(_ip_header_detail);
    std::string desc;
    std::ostringstream body;
    switch (data.d.type)
    {
    case 3:
    case 4:
    case 5:
    case 11:
    case 12:
        desc = "error";
        break;
    case 9:
    case 10:
        desc = "router";
        break;
    case 13:
    case 14: desc = "timestamp"; break;
    case ICMP_TYPE_PING_ASK:
        desc = "ping-ask";
        goto A;
    case ICMP_TYPE_PING_REPLY:
        desc = "ping-reply";
A:      if (icmp_len > sizeof(_icmp_header_detail)) {
            const char *c = reinterpret_cast<const char*>(&data) + sizeof(icmp_header);
            body << "\tPing Echo: " << std::string(c, icmp_len - sizeof(_icmp_header_detail)) << "\n";
        }
        break;
    case ICMP_TYPE_NETMASK_ASK:
        desc = "netmask-ask";
        goto B;
    case ICMP_TYPE_NETMASK_REPLY:
        desc = "netmask-reply";
B:      auto mh = reinterpret_cast<const icmp_netmask_header*>(&data);
        body << "\tICMP Netmask: " << mh->mask << "\n";
        break;
    }
    out << "\tICMP Type: " << desc << "(" << int(data.d.type) << ")" << "\n";
    out << "\tICMP Size: " << icmp_len << " bytes\n";
    out << "\tICMP Code: " << int(data.d.code) << "\n";
    out << "\tICMP Checksum: " << calc_checksum(&data.d, icmp_len) << "\n";
    out << "\tICMP Identification: " << ntohs(data.d.id) << "\n";
    out << "\tICMP Serial No.: " << ntohs(data.d.sn) << "\n";
    out << body.str();
    return out;
}