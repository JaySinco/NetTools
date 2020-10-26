#pragma once
#include <memory>
#include "protocol.h"

class packet
{
public:
    packet() = default;
    packet(const u_char *const start, const u_char *const end);
    void to_bytes(std::vector<u_char> &bytes) const;
    json to_json() const;

    static packet arp(const ip4 &dest);
    static packet arp(bool reverse, bool reply, const mac &smac, const ip4 &sip, const mac &dmac,
                      const ip4 &dip);

private:
    std::vector<std::shared_ptr<protocol>> stack;
};
