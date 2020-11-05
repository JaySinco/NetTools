#pragma once
#include "protocol.h"
#include <map>

class ethernet : public protocol
{
public:
    struct detail
    {
        mac dmac;      // Destination address
        mac smac;      // Source address
        u_short type;  // Ethernet type
    };

    ethernet() = default;

    ethernet(const u_char *const start, const u_char *&end, const protocol *prev = nullptr);

    ethernet(const mac &smac, const mac &dmac, const std::string &type);

    virtual ~ethernet() = default;

    virtual void to_bytes(std::vector<u_char> &bytes) const override;

    virtual json to_json() const override;

    virtual std::string type() const override;

    virtual std::string succ_type() const override;

    virtual bool link_to(const protocol &rhs) const override;

    const detail &get_detail() const;

private:
    detail d{0};

    static std::map<u_short, std::string> type_dict;

    static detail ntoh(const detail &d, bool reverse = false);

    static detail hton(const detail &d);
};
