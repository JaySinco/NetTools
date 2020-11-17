#pragma once
#include "type.h"
#include <vector>

#define Protocol_Type_Void "void"
#define Protocol_Type_Unknow(n) (fmt::format("unknow({:#x})", n))
#define Protocol_Type_Ethernet "ethernet"
#define Protocol_Type_IPv4 "ipv4"
#define Protocol_Type_IPv6 "ipv6"
#define Protocol_Type_ARP "arp"
#define Protocol_Type_RARP "rarp"
#define Protocol_Type_ICMP "icmp"
#define Protocol_Type_TCP "tcp"
#define Protocol_Type_UDP "udp"
#define Protocol_Type_DNS "dns"
#define Protocol_Type_HTTP "http"
#define Protocol_Type_SSH "ssh"

class protocol
{
public:
    // Destructor should be virtual
    virtual ~protocol() = default;

    // Serialize current protocol layer and insert in front of `bytes`, which contains
    // raw packet bytes serialized from higher layer.
    virtual void to_bytes(std::vector<u_char> &bytes) const = 0;

    // Encode protocol detail as json
    virtual json to_json() const = 0;

    // Self protocol type
    virtual std::string type() const = 0;

    // Successor protocol type that follows
    virtual std::string succ_type() const = 0;

    // Whether rhs is the response to this
    virtual bool link_to(const protocol &rhs) const = 0;

protected:
    static u_short calc_checksum(const void *data, size_t tlen);

    static u_short rand_ushort();

    static std::string guess_protocol_by_port(u_short port,
                                              const std::string &type = Protocol_Type_TCP);

private:
    static std::map<std::string, std::map<u_short, std::string>> port_dict;
};
