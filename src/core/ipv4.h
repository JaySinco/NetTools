#pragma once
#include "protocol.h"

class ipv4 : public protocol
{
public:
    struct detail
    {
        u_char ver_ihl;    // Version (4 bits) + Internet header length (4 bits)
        u_char tos;        // Type of service
        u_short tlen;      // Total length
        u_short id;        // Identification
        u_short flags_fo;  // Flags (3 bits) + Fragment offset (13 bits)
        u_char ttl;        // Time to live
        u_char type;       // IPv4 type
        u_short crc;       // Header checksum
        ip4 sip;           // Source address
        ip4 dip;           // Destination address
    };

    ipv4() = default;

    ipv4(const u_char *const start, const u_char *&end);

    ipv4(const ip4 &sip, const ip4 &dip, u_char ttl, const std::string &type);

    virtual ~ipv4();

    virtual void to_bytes(std::vector<u_char> &bytes) const override;

    virtual json to_json() const override;

    virtual std::string type() const override;

    virtual std::string succ_type() const override;

    virtual bool link_to(const protocol &rhs) const override;

    const detail &get_detail() const;

private:
    detail d{0};
    static std::map<u_char, std::string> type_dict;

    static detail ntoh(const detail &d, bool reverse = false);

    static detail hton(const detail &d);
};
