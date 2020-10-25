#include "ethernet.h"
#include <sstream>

bool mac::operator==(const mac &rhs) const
{
    return b1 == rhs.b1 && b2 == rhs.b2 && b3 == rhs.b3 && b4 == rhs.b4 && b5 == rhs.b5 &&
           b6 == rhs.b6;
}

bool mac::operator!=(const mac &rhs) const { return !(*this == rhs); }

std::string mac::to_str() const
{
    auto c = reinterpret_cast<const u_char *>(this);
    std::ostringstream ss;
    ss << std::hex << int(c[0]);
    for (int i = 1; i < 6; ++i) {
        ss << "-" << int(c[i]);
    }
    return ss.str();
}

std::map<u_short, std::string> ethernet::type_dict = {
    {0x0800, Protocol_Type_IPv4},
    {0x86dd, Protocol_Type_IPv6},
    {0x0806, Protocol_Type_ARP},
    {0x8035, Protocol_Type_RARP},
};

ethernet::ethernet(const u_char *start, const u_char *&end)
{
    d = *reinterpret_cast<const detail *>(start);
    end = start + sizeof(detail);
}

ethernet::ethernet(const mac &dest, const mac &source, const std::string &type)
{
    bool found = false;
    for (auto it = type_dict.cbegin(); it != type_dict.cend(); ++it) {
        if (it->second == type) {
            found = true;
            d.type = htons(it->first);
            break;
        }
    }
    if (!found) {
        throw std::runtime_error(fmt::format("unimplemented ethernet type: {}", type));
    }
    d.dest = dest;
    d.source = source;
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
    j["self"] = type();
    j["type"] = succ_type();
    j["source-mac"] = d.source.to_str();
    j["dest-mac"] = d.dest.to_str();
    return j;
}

std::string ethernet::type() const { return Protocol_Type_Ethernet; }

std::string ethernet::succ_type() const
{
    u_short type = ntohs(d.type);
    if (type_dict.count(type) != 0) {
        return type_dict[type];
    }
    return Protocol_Type_Unimplemented(type);
}
