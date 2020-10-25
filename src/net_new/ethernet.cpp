#include "ethernet.h"

bool mac::operator==(const mac &rhs) const
{
    return b1 == rhs.b1 && b2 == rhs.b2 && b3 == rhs.b3 && b4 == rhs.b4 && b5 == rhs.b5 &&
           b6 == rhs.b6;
}

bool mac::operator!=(const mac &rhs) const { return !(*this == rhs); }

ethernet::ethernet(const u_char *start, const u_char *&end)
{
    d = *reinterpret_cast<const detail *>(start);
    end = start + sizeof(detail);
}

ethernet::~ethernet() {}

void ethernet::to_bytes(std::vector<u_char> &bytes) const
{
    auto it = reinterpret_cast<const u_char *>(&d);
    bytes.insert(bytes.cbegin(), it, it + sizeof(detail));
}

json ethernet::to_json() const
{
    json j;
    return j;
}
