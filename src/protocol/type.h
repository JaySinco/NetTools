#pragma once
#include "common.h"

struct mac
{
    u_char b1, b2, b3, b4, b5, b6;

public:
    bool operator==(const mac &rhs) const;

    bool operator!=(const mac &rhs) const;

    std::string to_str() const;

    static const mac zeros;

    static const mac broadcast;
};

struct ip4
{
    u_char b1, b2, b3, b4;

public:
    ip4() = default;

    ip4(const ip4 &) = default;

    ip4(u_char c1, u_char c2, u_char c3, u_char c4);

    explicit ip4(const std::string &s);

    ip4(const in_addr &addr);

    operator in_addr() const;

    operator u_int() const;

    bool operator==(const ip4 &rhs) const;

    bool operator!=(const ip4 &rhs) const;

    u_int operator&(const ip4 &rhs) const;

    bool is_local(const ip4 &rhs, const ip4 &mask) const;

    std::string to_str() const;

    static bool from_dotted_dec(const std::string &s, ip4 *ip = nullptr);

    static bool from_domain(const std::string &s, ip4 *ip = nullptr);

    static const ip4 zeros;

    static const ip4 broadcast;
};

struct wsa_guard
{
    wsa_guard();
    ~wsa_guard();

private:
    static wsa_guard g;
};
