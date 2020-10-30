#pragma once
#include "protocol.h"

class icmp : public protocol
{
public:
    struct detail
    {
        u_char type;  // Type
        u_char code;  // Code
        u_short crc;  // Checksum as a whole
        union
        {
            struct
            {
                u_short id;  // Identification
                u_short sn;  // Serial number
            } s;
            u_int i;
        } u;
    };

    icmp() = default;

    icmp(const u_char *const start, const u_char *&end);

    virtual ~icmp();

    virtual void to_bytes(std::vector<u_char> &bytes) const override;

    virtual json to_json() const override;

    virtual std::string type() const override;

    virtual std::string succ_type() const override;

    virtual bool link_to(const protocol &rhs) const override;

    const detail &get_detail() const;

private:
    detail d{0};
    std::vector<u_char> extra;

    static detail ntoh(const detail &d, bool reverse = false);

    static detail hton(const detail &d);
};
