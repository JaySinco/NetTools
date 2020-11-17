#include "protocol.h"
#include <random>

std::map<std::string, std::map<u_short, std::string>> protocol::port_dict = {
    {Protocol_Type_UDP,
     {{22, Protocol_Type_SSH}, {53, Protocol_Type_DNS}, {80, Protocol_Type_HTTP}}},
    {Protocol_Type_TCP,
     {{22, Protocol_Type_SSH}, {53, Protocol_Type_DNS}, {80, Protocol_Type_HTTP}}}};

u_short protocol::calc_checksum(const void *data, size_t tlen)
{
    uint32_t sum = 0;
    auto buf = static_cast<const u_short *>(data);
    while (tlen > 1) {
        sum += *buf++;
        tlen -= 2;
    }
    if (tlen > 0) {
        u_short left = 0;
        std::memcpy(&left, buf, 1);
        sum += left;
    }
    while (sum >> 16) {
        sum = (sum & 0xffff) + (sum >> 16);
    }
    return (static_cast<u_short>(sum) ^ 0xffff);
}

u_short protocol::rand_ushort()
{
    static std::random_device rd;
    static std::default_random_engine engine(rd());
    static std::uniform_int_distribution<u_short> dist;
    return dist(engine);
}

std::string protocol::guess_protocol_by_port(u_short port, const std::string &type)
{
    if (port_dict.count(type) > 0) {
        auto &type_dict = port_dict.at(type);
        if (type_dict.count(port) > 0) {
            return type_dict.at(port);
        }
    }
    return Protocol_Type_Void;
}
