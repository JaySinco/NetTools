#pragma once
#include "protocol.h"

class tcp : public protocol
{
public:
    struct detail
    {
        u_short sport;     // Source port
        u_short dport;     // Destination port
        u_int sn;          // Sequence number
        u_int an;          // Acknowledgment number
        u_short hl_flags;  // Header length (4 bits) + Reserved (3 bits) + Flags (9 bits)
        u_short wlen;      // Window Size
        u_short crc;       // Checksum
        u_short urp;       // Urgent pointer
    };

    struct extra_detail
    {
        u_short len;  // Datagram length, >= 20
        u_short crc;  // Computed checksum
    };

    struct pseudo_header
    {
        ip4 sip;          // IPv4 Source address
        ip4 dip;          // IPv4 Destination address
        u_char zero_pad;  // Zero
        u_char type;      // IPv4 type
        u_short len;      // TCP Datagram length
    };

    tcp() = default;

    tcp(const u_char *const start, const u_char *&end, const protocol *prev);

    virtual ~tcp() = default;

    virtual void to_bytes(std::vector<u_char> &bytes) const override;

    virtual json to_json() const override;

    virtual std::string type() const override;

    virtual std::string succ_type() const override;

    virtual bool link_to(const protocol &rhs) const override;

    const detail &get_detail() const;

private:
    detail d{0};

    extra_detail extra;

    static detail ntoh(const detail &d, bool reverse = false);

    static detail hton(const detail &d);
};
