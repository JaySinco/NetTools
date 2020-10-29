#pragma once
#include <memory>
#include <map>
#include "protocol.h"

class packet
{
public:
    struct detail
    {
        std::vector<std::shared_ptr<protocol>> layers;  // Protocol layers
        tm recv_tm = {0};                               // Received time roughly
        long recv_ms;                                   // Received time in ms
    };

    packet() = default;
    packet(const u_char *const start, const u_char *const end, long recv_sec = 0, long ms = 0);
    void to_bytes(std::vector<u_char> &bytes) const;
    json to_json() const;
    bool link_to(const packet &rhs) const;
    const detail &get_detail() const;

    static packet arp(const ip4 &dest);
    static packet arp(const mac &smac, const ip4 &sip, const mac &dmac, const ip4 &dip,
                      bool reply = false, bool reverse = false);

private:
    detail d;

    using decoder = std::shared_ptr<protocol> (*)(const u_char *const start, const u_char *&end);
    static std::map<std::string, decoder> decoder_dict;

    template <typename T>
    static std::shared_ptr<protocol> decode(const u_char *const start, const u_char *&end)
    {
        return std::make_shared<T>(start, end);
    }
};
