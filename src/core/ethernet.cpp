#include "ethernet.h"

std::map<u_short, std::string> ethernet::type_dict = {
    {0x0800, Protocol_Type_IPv4},
    {0x86dd, Protocol_Type_IPv6},
    {0x0806, Protocol_Type_ARP},
    {0x8035, Protocol_Type_RARP},
};

ethernet::ethernet(const u_char *const start, const u_char *&end)
{
    d = convert_detail(true, *reinterpret_cast<const detail *>(start));
    end = start + sizeof(detail);
}

ethernet::ethernet(const mac &dest, const mac &source, const std::string &type)
{
    bool found = false;
    for (auto it = type_dict.cbegin(); it != type_dict.cend(); ++it) {
        if (it->second == type) {
            found = true;
            d.type = it->first;
            break;
        }
    }
    if (!found) {
        throw std::runtime_error(fmt::format("unknow ethernet type: {}", type));
    }
    d.dest = dest;
    d.source = source;
}

ethernet::~ethernet() {}

void ethernet::to_bytes(std::vector<u_char> &bytes) const
{
    auto dt = convert_detail(false, d);
    auto it = reinterpret_cast<const u_char *>(&dt);
    bytes.insert(bytes.cbegin(), it, it + sizeof(detail));
}

json ethernet::to_json() const
{
    json j;
    j["type"] = type();
    j["succ-type"] = succ_type();
    j["source-mac"] = d.source.to_str();
    j["dest-mac"] = d.dest.to_str();
    return j;
}

std::string ethernet::type() const { return Protocol_Type_Ethernet; }

std::string ethernet::succ_type() const
{
    if (type_dict.count(d.type) != 0) {
        return type_dict[d.type];
    }
    return Protocol_Type_Unknow(d.type);
}

bool ethernet::link_to(const protocol &rhs) const
{
    if (type() == rhs.type()) {
        auto p = dynamic_cast<const ethernet &>(rhs);
        return p.d.dest == mac::broadcast || d.source == p.d.dest;
    }
    return false;
}

const ethernet::detail &ethernet::get_detail() const { return d; }

ethernet::detail ethernet::convert_detail(bool ntoh, const detail &d)
{
    detail dt = d;
    NET_CVT(dt.type, ntoh, s);
    return dt;
}
