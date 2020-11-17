#include "udp.h"
#include "ipv4.h"

udp::udp(const u_char *const start, const u_char *&end, const protocol *prev)
{
    d = ntoh(*reinterpret_cast<const detail *>(start));
    end = start + sizeof(detail);
    auto &ipdt = dynamic_cast<const ipv4 *>(prev)->get_detail();
    pseudo_header ph;
    ph.sip = ipdt.sip;
    ph.dip = ipdt.dip;
    ph.type = ipdt.type;
    ph.zero_pad = 0;
    ph.len = htons(d.len);
    std::string raw = std::string(start + sizeof(detail), start + d.len);
    size_t tlen = sizeof(pseudo_header) + sizeof(detail) + raw.size();
    u_char *buf = new u_char[tlen];
    auto dt = hton(d);
    std::memcpy(buf, &ph, sizeof(pseudo_header));
    std::memcpy(buf + sizeof(pseudo_header), &dt, sizeof(detail));
    std::memcpy(buf + sizeof(pseudo_header) + sizeof(detail), raw.data(), raw.size());
    extra.crc = calc_checksum(buf, tlen);
    delete[] buf;
}

void udp::to_bytes(std::vector<u_char> &bytes) const
{
    throw std::runtime_error("unimplemented method");
}

json udp::to_json() const
{
    json j;
    j["type"] = type();
    j["udp-type"] = succ_type();
    j["source-port"] = d.sport;
    j["dest-port"] = d.dport;
    j["total-size"] = d.len;
    j["checksum"] = extra.crc;
    return j;
}

std::string udp::type() const { return Protocol_Type_UDP; }

std::string udp::succ_type() const
{
    std::string dtype = guess_protocol_by_port(d.dport, Protocol_Type_UDP);
    if (dtype != Protocol_Type_Void) {
        return dtype;
    }
    return guess_protocol_by_port(d.sport, Protocol_Type_UDP);
}

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
