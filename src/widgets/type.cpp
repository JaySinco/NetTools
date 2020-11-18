#include "type.h"

wxColour hashed_color(const std::string &data)
{
    std::hash<std::string> hash_func;
    size_t hash = hash_func(data);
    auto p = reinterpret_cast<unsigned char *>(&hash);
    return wxColour(p[0], p[1], p[2]);
}

wxColour hashed_color(ip4 a, ip4 b)
{
    if (a > b) {
        std::swap(a, b);
    }
    char buf[sizeof(ip4) * 2];
    std::memcpy(buf, &a, sizeof(ip4));
    std::memcpy(buf + sizeof(ip4), &b, sizeof(ip4));
    return hashed_color(std::string(buf, sizeof(buf)));
}
