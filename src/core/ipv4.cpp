#include "ipv4.h"

std::map<u_char, std::string> ipv4::type_dict = {
    {1, Protocol_Type_ICMP},
    {6, Protocol_Type_TCP},
    {17, Protocol_Type_UDP},
};

ipv4::ipv4(const u_char *const start, const u_char *&end)
{
    d = ntoh(*reinterpret_cast<const detail *>(start));
    assert(d.tlen == (end - start));
    end = start + 4 * (d.ver_ihl & 0xf);
}

ipv4::~ipv4() {}

void ipv4::to_bytes(std::vector<u_char> &bytes) const
{
    d.ver_ihl = (4 << 4) | (sizeof(detail) / 4);
    d.tlen = sizeof(detail) + bytes.size();
    d.crc = calc_checksum(&d, sizeof(detail));

    auto dt = hton(d);
    auto it = reinterpret_cast<const u_char *>(&dt);
    bytes.insert(bytes.cbegin(), it, it + sizeof(detail));
}

json ipv4::to_json() const
{
    json j;
    j["type"] = type();
    j["succ-type"] = succ_type();
    j["version"] = d.ver_ihl >> 4;
    size_t header_size = 4 * (d.ver_ihl & 0xf);
    j["header-size"] = header_size;
    int checksum = -1;
    if (header_size == sizeof(detail)) {
        auto dt = hton(d);
        checksum = calc_checksum(&dt, header_size);
    }
    j["header-checksum"] = checksum;
    j["total-size"] = d.tlen;
    j["id"] = d.id;
    j["more-fragment"] = d.flags_fo & 0x2000 ? true : false;
    j["not-slice"] = d.flags_fo & 0x4000 ? true : false;
    j["fragment-offset"] = (d.flags_fo & 0x1fff) * 8;
    j["ttl"] = static_cast<int>(d.ttl);
    j["source-ip"] = d.sip.to_str();
    j["dest-ip"] = d.dip.to_str();
    return j;
}

std::string ipv4::type() const { return Protocol_Type_IPv4; }

std::string ipv4::succ_type() const
{
    if (type_dict.count(d.proto) != 0) {
        return type_dict[d.proto];
    }
    return Protocol_Type_Unknow(d.proto);
}

bool ipv4::link_to(const protocol &rhs) const
{
    if (type() == rhs.type()) {
        auto p = dynamic_cast<const ipv4 &>(rhs);
        return d.sip == p.d.dip;
    }
    return false;
}

const ipv4::detail &ipv4::get_detail() const { return d; }

ipv4::detail ipv4::hton(const detail &d) { return ntoh(d, true); }

ipv4::detail ipv4::ntoh(const detail &d, bool reverse)
{
    detail dt = d;
    NET_CVT(dt.tlen, !reverse, s);
    NET_CVT(dt.id, !reverse, s);
    NET_CVT(dt.flags_fo, !reverse, s);
    return dt;
}
