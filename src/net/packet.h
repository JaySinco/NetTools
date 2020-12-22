#pragma once
#include <memory>
#include <map>
#include <optional>
#include "protocol.h"

class packet
{
public:
    struct detail
    {
        timeval time;                                   // Received time
        std::vector<std::shared_ptr<protocol>> layers;  // Protocol layers
        std::string owner;                              // Program that generates this packet
    };

    packet();

    packet(const u_char *const start, const u_char *const end, const timeval &tv);

    void to_bytes(std::vector<u_char> &bytes) const;

    json to_json() const;

    const json &to_json_flat() const;

    bool link_to(const packet &rhs) const;

    const detail &get_detail() const;

    void set_time(const timeval &tv);

    bool is_error() const;

    bool has_type(const std::string &type) const;

    static timeval gettimeofday();

    static packet arp(const mac &smac, const ip4 &sip, const mac &dmac, const ip4 &dip,
                      bool reply = false, bool reverse = false);

    static packet ping(const mac &smac, const ip4 &sip, const mac &dmac, const ip4 &dip,
                       u_char ttl = 128, const std::string &echo = "", bool forbid_slice = false);

private:
    detail d;

    std::optional<json> j_flat;

    std::string get_owner() const;

    using decoder = std::shared_ptr<protocol> (*)(const u_char *const start, const u_char *&end,
                                                  const protocol *prev);

    static std::map<std::string, decoder> decoder_dict;

    template <typename T>
    static std::shared_ptr<protocol> decode(const u_char *const start, const u_char *&end,
                                            const protocol *prev)
    {
        return std::make_shared<T>(start, end, prev);
    }
};
