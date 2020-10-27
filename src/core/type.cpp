#include "type.h"
#include <ws2tcpip.h>
#include <sstream>

const mac mac::zeros = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
const mac mac::broadcast = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

bool mac::operator==(const mac &rhs) const
{
    return b1 == rhs.b1 && b2 == rhs.b2 && b3 == rhs.b3 && b4 == rhs.b4 && b5 == rhs.b5 &&
           b6 == rhs.b6;
}

bool mac::operator!=(const mac &rhs) const { return !(*this == rhs); }

std::string mac::to_str() const
{
    auto c = reinterpret_cast<const u_char *>(this);
    std::ostringstream ss;
    ss << std::hex << int(c[0]);
    for (int i = 1; i < 6; ++i) {
        ss << "-" << int(c[i]);
    }
    return ss.str();
}

const ip4 ip4::zeros = {0x0, 0x0, 0x0, 0x0};
const ip4 ip4::broadcast = {0xff, 0xff, 0xff, 0xff};

ip4::ip4(u_char c1, u_char c2, u_char c3, u_char c4) : b1(c1), b2(c2), b3(c3), b4(c4) {}

ip4::ip4(const std::string &s)
{
    if (!from_dotted_dec(s, this)) {
        throw std::runtime_error(fmt::format("invalid ip4: {}", s));
    }
}

ip4::ip4(const in_addr &addr)
{
    b1 = addr.S_un.S_un_b.s_b1;
    b2 = addr.S_un.S_un_b.s_b2;
    b3 = addr.S_un.S_un_b.s_b3;
    b4 = addr.S_un.S_un_b.s_b4;
}

ip4::operator in_addr() const
{
    in_addr addr;
    addr.S_un.S_un_b.s_b1 = b1;
    addr.S_un.S_un_b.s_b2 = b2;
    addr.S_un.S_un_b.s_b3 = b3;
    addr.S_un.S_un_b.s_b4 = b4;
    return addr;
}

ip4::operator u_int() const
{
    auto i = reinterpret_cast<const u_int *>(this);
    return *i;
}

bool ip4::operator==(const ip4 &rhs) const
{
    return b1 == rhs.b1 && b2 == rhs.b2 && b3 == rhs.b3 && b4 == rhs.b4;
}

bool ip4::operator!=(const ip4 &rhs) const { return !(*this == rhs); }

u_int ip4::operator&(const ip4 &rhs) const
{
    auto i = reinterpret_cast<const u_int *>(this);
    auto j = reinterpret_cast<const u_int *>(&rhs);
    return ntohl(*i) & ntohl(*j);
}

bool ip4::is_local(const ip4 &rhs, const ip4 &mask) const { return (*this & mask) == (rhs & mask); }

bool ip4::from_dotted_dec(const std::string &s, ip4 *ip)
{
    in_addr addr;
    if (inet_pton(AF_INET, s.c_str(), &addr) != 1) {
        return false;
    }
    if (ip) {
        *ip = ip4(addr);
    }
    return true;
}

bool ip4::from_domain(const std::string &s, ip4 *ip)
{
    addrinfo hints = {0};
    hints.ai_family = AF_INET;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = 0;
    hints.ai_socktype = SOCK_STREAM;
    addrinfo *first_addr;
    auto ret = GetAddrInfoA(s.c_str(), nullptr, &hints, &first_addr);
    if (ret != 0 || first_addr == nullptr) {
        return false;
    }
    if (ip) {
        *ip = reinterpret_cast<sockaddr_in *>(first_addr->ai_addr)->sin_addr;
    }
    return true;
}

std::string ip4::to_str() const
{
    auto c = reinterpret_cast<const u_char *>(this);
    std::ostringstream ss;
    ss << int(c[0]);
    for (int i = 1; i < 4; ++i) {
        ss << "." << int(c[i]);
    }
    return ss.str();
}

json adapter::to_json() const
{
    json j;
    j["name"] = name;
    j["desc"] = desc;
    j["mac"] = mac_.to_str();
    j["ip"] = ip.to_str();
    j["mask"] = mask.to_str();
    j["gateway"] = gateway.to_str();
    return j;
}
