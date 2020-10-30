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
        timeval time;                                   // Received time
    };

    packet();

    packet(const u_char *const start, const u_char *const end, const timeval &tv);

    void to_bytes(std::vector<u_char> &bytes) const;

    json to_json() const;

    bool link_to(const packet &rhs) const;

    const detail &get_detail() const;

    static timeval gettimeofday();

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
