#pragma once
#include <winsock2.h>
#include <vector>
#include <json.hpp>
#include <fmt/core.h>

#define Protocol_Type_Unimplemented(n) fmt::format("unimplemented({:#x})", n)
#define Protocol_Type_Ethernet "ethernet"
#define Protocol_Type_IPv4 "ipv4"
#define Protocol_Type_IPv6 "ipv6"
#define Protocol_Type_ARP "arp"
#define Protocol_Type_RARP "rarp"

using json = nlohmann::json;

class protocol
{
public:
    // Destructor should be virtual
    virtual ~protocol();

    // Serialize current protocol layer and insert in front of `bytes`, which contains
    // raw packet bytes serialized from higher layer.
    virtual void to_bytes(std::vector<u_char> &bytes) const;

    // Encode protocol detail as json
    virtual json to_json() const;

    // Self protocol type
    virtual std::string type() const;

    // Successor protocol type that follows
    virtual std::string succ_type() const;
};
