#pragma once
#include "protocol.h"

struct mac
{
    u_char b1, b2, b3, b4, b5, b6;

    bool operator==(const mac &rhs) const;
    bool operator!=(const mac &rhs) const;
};

class ethernet : public protocol
{
public:
    ethernet() = default;
    ethernet(const u_char *start, const u_char *&end);
    virtual ~ethernet();

    virtual void to_bytes(std::vector<u_char> &bytes) const override;
    virtual json to_json() const override;

private:
    struct detail
    {
        mac dea;           // Destination address
        mac sea;           // Source address
        u_short eth_type;  // Ethernet type
    } d;
};
