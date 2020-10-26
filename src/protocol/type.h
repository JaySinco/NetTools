#pragma once
#include <winsock2.h>
#include <fmt/core.h>

#define Protocol_Type_Void "void"
#define Protocol_Type_Unimplemented(n) fmt::format("unimplemented({:#x})", n)
#define Protocol_Type_Ethernet "ethernet"
#define Protocol_Type_IPv4 "ipv4"
#define Protocol_Type_IPv6 "ipv6"
#define Protocol_Type_ARP "arp"
#define Protocol_Type_RARP "rarp"

struct mac
{
    u_char b1, b2, b3, b4, b5, b6;

    bool operator==(const mac &rhs) const;
    bool operator!=(const mac &rhs) const;

    std::string to_str() const;

    static const mac placeholder;
    static const mac broadcast;
};

struct ip4
{
    u_char b1, b2, b3, b4;

    ip4() = default;
    ip4(u_char c1, u_char c2, u_char c3, u_char c4);
    ip4(const in_addr &addr);

    operator in_addr() const;
    operator u_int() const;
    bool operator==(const ip4 &rhs) const;
    bool operator!=(const ip4 &rhs) const;
    u_int operator&(const ip4 &rhs) const;

    bool is_local(const ip4 &rhs, const ip4 &netmask) const;
    std::string to_str() const;

    static bool from_dotted_dec(const std::string &s, ip4 *ip = nullptr);
    static bool from_domain(const std::string &s, ip4 *ip = nullptr);
    static const ip4 placeholder;
    static const ip4 broadcast;
};
