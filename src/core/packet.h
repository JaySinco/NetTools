#pragma once
#include <memory>
#include <map>
#include "protocol.h"

class packet
{
public:
    packet() = default;
    packet(const u_char *const start, const u_char *const end);
    void to_bytes(std::vector<u_char> &bytes) const;
    json to_json() const;
    bool link_to(const packet &rhs) const;
    void received_at(long sec, long ms);

    static packet arp(const ip4 &dest);
    static packet arp(bool reverse, bool reply, const mac &smac, const ip4 &sip, const mac &dmac,
                      const ip4 &dip);

private:
    std::vector<std::shared_ptr<protocol>> layers;
    tm recv_tm = {0};
    long recv_ms;

    using decoder = std::shared_ptr<protocol> (*)(const u_char *const start, const u_char *&end);
    static std::map<std::string, decoder> decoder_dict;

    template <typename T>
    static std::shared_ptr<protocol> decode(const u_char *const start, const u_char *&end)
    {
        return std::make_shared<T>(start, end);
    }
};
