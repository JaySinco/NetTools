#include "tcp.h"
#include "ipv4.h"

tcp::tcp(const u_char *const start, const u_char *&end, const protocol *prev)
{
    d = ntoh(*reinterpret_cast<const detail *>(start));
    end = start + 4 * (d.thl_flags >> 12 & 0xf);
}

void tcp::to_bytes(std::vector<u_char> &bytes) const
{
    throw std::runtime_error("unimplemented method");
}

json tcp::to_json() const
{
    json j;
    j["type"] = type();
    j["tcp-type"] = succ_type();
    j["source-port"] = d.sport;
    j["dest-port"] = d.dport;
    j["sequence-num"] = d.sn;
    j["acknowledge-num"] = d.an;
    size_t header_size = 4 * (d.thl_flags >> 12 & 0xf);
    j["header-size"] = header_size;
    return j;
}

std::string tcp::type() const { return Protocol_Type_TCP; }

std::string tcp::succ_type() const
{
    std::string dtype = guess_protocol_by_port(d.dport, Protocol_Type_TCP);
    if (is_specific(dtype)) {
        return dtype;
    }
    return guess_protocol_by_port(d.sport, Protocol_Type_TCP);
}

bool tcp::link_to(const protocol &rhs) const
{
    if (type() == rhs.type()) {
        auto p = dynamic_cast<const tcp &>(rhs);
        return d.sport == p.d.dport && d.dport == p.d.sport;
    }
    return false;
}

const tcp::detail &tcp::get_detail() const { return d; }

tcp::detail tcp::ntoh(const detail &d, bool reverse)
{
    detail dt = d;
    ntohx(dt.sport, !reverse, s);
    ntohx(dt.dport, !reverse, s);
    ntohx(dt.sn, !reverse, l);
    ntohx(dt.an, !reverse, l);
    ntohx(dt.thl_flags, !reverse, s);
    ntohx(dt.wlen, !reverse, s);
    ntohx(dt.urp, !reverse, s);
    return dt;
}

tcp::detail tcp::hton(const detail &d) { return ntoh(d, true); }
