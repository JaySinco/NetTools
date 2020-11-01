#include "protocol.h"
#include <random>

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
