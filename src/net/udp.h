#pragma once
#include "protocol.h"

class udp : public protocol
{
public:
    struct detail
    {
        u_short sport;  // Source port
        u_short dport;  // Destination port
        u_short len;    // Datagram length, >= 8
        u_short crc;    // Checksum
    };

    struct extra_detail
    {
        u_short crc;  // Computed checksum
    };

    struct pseudo_header
    {
        ip4 sip;          // IPv4 Source address
        ip4 dip;          // IPv4 Destination address
        u_char zero_pad;  // Zero
        u_char type;      // IPv4 type
        u_short len;      // UDP Datagram length
    };

    udp() = default;

    udp(const u_char *const start, const u_char *&end, const protocol *prev);

    virtual ~udp() = default;

    virtual void to_bytes(std::vector<u_char> &bytes) const override;

    virtual json to_json() const override;

    virtual std::string type() const override;

    virtual std::string succ_type() const override;

    virtual bool link_to(const protocol &rhs) const override;

    const detail &get_detail() const;

    const extra_detail &get_extra() const;

private:
    detail d{0};

    extra_detail extra;

    static detail ntoh(const detail &d, bool reverse = false);

    static detail hton(const detail &d);
};
