#pragma once
#include "protocol.h"
#include "ipv4.h"

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

    struct extra_detail
    {
        std::string raw;  // Raw data, including ping echo
        ipv4 eip;         // Error ip header
        u_char buf[8];    // At least 8-bit behind ip header
    };

    icmp() = default;

    icmp(const u_char *const start, const u_char *&end, const protocol *prev);

    icmp(const std::string &ping_echo);

    virtual ~icmp() = default;

    virtual void to_bytes(std::vector<u_char> &bytes) const override;

    virtual json to_json() const override;

    virtual std::string type() const override;

    virtual std::string succ_type() const override;

    virtual bool link_to(const protocol &rhs) const override;

    const detail &get_detail() const;

    const extra_detail &get_extra() const;

    std::string icmp_type() const;

private:
    detail d{0};

    extra_detail extra;

    static std::map<u_char, std::pair<std::string, std::map<u_char, std::string>>> type_dict;

    static detail ntoh(const detail &d, bool reverse = false);

    static detail hton(const detail &d);
};
