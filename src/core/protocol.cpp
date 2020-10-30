#include "protocol.h"

u_short protocol::calc_checksum(const void *data, size_t tlen)
{
    bool odd = (tlen % 2 != 0);
    if (odd) {
        void *buf = new u_char[tlen + 1]{0};
        std::memcpy(buf, data, tlen);
        data = buf;
        tlen += 1;
    }
    u_long checksum = 0;
    auto check_ptr = reinterpret_cast<const u_short *>(data);
    for (int i = 0; i < tlen / 2; ++i) {
        checksum += check_ptr[i];
        checksum = (checksum >> 16) + (checksum & 0xffff);
    }
    if (odd) {
        delete[] data;
    }
    return static_cast<u_short>(~checksum);
}
