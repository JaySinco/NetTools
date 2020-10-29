#pragma once
#include "protocol.h"

class arp : public protocol
{
public:
    struct detail
    {
        u_short hw_type;    // Hardware type
        u_short prot_type;  // Protocol type
        u_char hw_len;      // Hardware address length
        u_char prot_len;    // Protocol address length
        u_short op;         // Operation code
        mac smac;           // Source ethernet address
        ip4 sip;            // Source ip address
        mac dmac;           // Destination ethernet address
        ip4 dip;            // Destination ip address
    };

    arp() = default;
    arp(const u_char *const start, const u_char *&end);
    arp(const mac &smac, const ip4 &sip, const mac &dmac, const ip4 &dip, bool reply = false,
        bool reverse = false);
    virtual ~arp();

    virtual void to_bytes(std::vector<u_char> &bytes) const override;
    virtual json to_json() const override;
    virtual std::string type() const override;
    virtual std::string succ_type() const override;
    virtual bool link_to(const protocol &rhs) const override;

    const detail &get_detail() const;

private:
    detail d;
};
