#pragma once
#include <memory>
#include <functional>
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

    static packet arp(const ip4 &dest);
    static packet arp(bool reverse, bool reply, const mac &smac, const ip4 &sip, const mac &dmac,
                      const ip4 &dip);

private:
    std::vector<std::shared_ptr<protocol>> stack;

    using decoder_type =
        std::function<std::shared_ptr<protocol>(const u_char *const start, const u_char *&end)>;

    template <typename T>
    static std::shared_ptr<protocol> decoder(const u_char *const start, const u_char *&end)
    {
        return std::make_shared<T>(pstart, pend);
    }

    static std::map<std::string, decoder_type> decoder_dict;
};
