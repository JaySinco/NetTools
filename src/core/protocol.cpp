#include "protocol.h"

u_short protocol::calc_checksum(const void *data, size_t tlen)
{
    uint32_t sum = 0;
    auto buf = static_cast<const u_char *>(data);
    while (tlen > 1) {
        sum += 0xffff & (*buf << 8 | *(buf + 1));
        buf += 2;
        tlen -= 2;
    }
    if (tlen > 0) {
        sum += 0xffff & (*buf << 8 | 0x00);
    }
    while (sum >> 16) {
        sum = (sum & 0xffff) + (sum >> 16);
    }
    return ((u_short)sum ^ 0xffff);
}
