#include "udp.h"
#include "ipv4.h"

udp::udp(const u_char *const start, const u_char *&end, const protocol *prev)
{
    d = ntoh(*reinterpret_cast<const detail *>(start));
    extra.raw = std::string(start + sizeof(detail), start + d.len);
    auto &ipdt = dynamic_cast<const ipv4 *>(prev)->get_detail();
    ph.sip = ipdt.sip;
    ph.dip = ipdt.dip;
    ph.type = ipdt.type;
    ph.zero_pad = 0;
    ph.len = htons(d.len);
}

void udp::to_bytes(std::vector<u_char> &bytes) const
{
    throw std::runtime_error("unimplemented method");
}

json udp::to_json() const
{
    json j;
    j["type"] = type();
    j["source-port"] = d.sport;
    j["dest-port"] = d.dport;
    j["total-size"] = d.len;
    size_t tlen = sizeof(pseudo_header) + sizeof(detail) + extra.raw.size();
    u_char *buf = new u_char[tlen];
    auto dt = hton(d);
    std::memcpy(buf, &ph, sizeof(pseudo_header));
    std::memcpy(buf + sizeof(pseudo_header), &dt, sizeof(detail));
    std::memcpy(buf + sizeof(pseudo_header) + sizeof(detail), extra.raw.data(), extra.raw.size());
    j["checksum"] = calc_checksum(buf, tlen);
    delete[] buf;
    return j;
}

std::string udp::type() const { return Protocol_Type_UDP; }

std::string udp::succ_type() const { return Protocol_Type_Void; }

bool udp::link_to(const protocol &rhs) const
{
    if (type() == rhs.type()) {
        auto p = dynamic_cast<const udp &>(rhs);
        return d.sport == p.d.dport && d.dport == p.d.sport;
    }
    return false;
}

const udp::detail &udp::get_detail() const { return d; }

const udp::extra_detail &udp::get_extra() const { return extra; }

udp::detail udp::ntoh(const detail &d, bool reverse)
{
    detail dt = d;
    ntoh_cvt(dt.sport, !reverse, s);
    ntoh_cvt(dt.dport, !reverse, s);
    ntoh_cvt(dt.len, !reverse, s);
    return dt;
}

udp::detail udp::hton(const detail &d) { return ntoh(d, true); }
