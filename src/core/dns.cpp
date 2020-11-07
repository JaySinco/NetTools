#include "dns.h"

dns::dns(const u_char *const start, const u_char *&end, const protocol *prev)
{
    d = ntoh(*reinterpret_cast<const detail *>(start));
    auto it = start + sizeof(detail);
    for (int i = 0; i < d.qrn; ++i) {
        query_detail qr;
        qr.domain = decode_domain(start, end, it);
        qr.type = ntohs(*reinterpret_cast<const u_short *>(it));
        it += sizeof(u_short);
        qr.cls = ntohs(*reinterpret_cast<const u_short *>(it));
        it += sizeof(u_short);
        extra.query.push_back(qr);
    }
    auto parse_res = [&](std::vector<res_detail> &vec, u_short count) {
        for (int i = 0; i < count; ++i) {
            res_detail rr;
            rr.domain = decode_domain(start, end, it);
            rr.type = ntohs(*reinterpret_cast<const u_short *>(it));
            it += sizeof(u_short);
            rr.cls = ntohs(*reinterpret_cast<const u_short *>(it));
            it += sizeof(u_short);
            rr.ttl = ntohl(*reinterpret_cast<const u_int *>(it));
            it += sizeof(u_int);
            rr.data_len = ntohs(*reinterpret_cast<const u_short *>(it));
            it += sizeof(u_short);
            if (rr.type == 5) {  // CNAME
                std::string alias = decode_domain(start, end, it);
                rr.res_data.insert(rr.res_data.cend(), alias.cbegin(), alias.cend());
            } else {
                rr.res_data = std::string(it, it + rr.data_len);
                it += rr.data_len;
            }
            vec.push_back(rr);
        }
    };
    parse_res(extra.reply, d.rrn);
    parse_res(extra.auth, d.arn);
    parse_res(extra.extra, d.ern);
}

void dns::to_bytes(std::vector<u_char> &bytes) const
{
    throw std::runtime_error("unimplemented method");
}

json dns::to_json() const
{
    json j;
    j["type"] = type();
    j["id"] = d.id;
    return j;
}

std::string dns::type() const { return Protocol_Type_DNS; }

std::string dns::succ_type() const { return Protocol_Type_Void; }

bool dns::link_to(const protocol &rhs) const
{
    if (type() == rhs.type()) {
        auto p = dynamic_cast<const dns &>(rhs);
        return d.id == p.d.id;
    }
    return false;
}

const dns::detail &dns::get_detail() const { return d; }

const dns::extra_detail &dns::get_extra() const { return extra; }

std::string dns::encode_domain(const std::string &domain)
{
    std::string bytes;
    auto svec = string_split(domain, ".");
    for (const auto &s : svec) {
        if (s.size() > 63) {
            throw std::runtime_error(fmt::format("segment of domain exceed 63: {}", s));
        }
        bytes.push_back(static_cast<u_char>(s.size()));
        bytes.insert(bytes.end(), s.cbegin(), s.cend());
    }
    bytes.push_back(0);
    return bytes;
}

std::string dns::decode_domain(const u_char *const pstart, const u_char *const pend,
                               const u_char *&it)
{
    std::vector<std::string> domain_vec;
    bool compressed = false;
    for (; it < pend && *it != 0;) {
        size_t cnt = *it;
        if ((cnt & 0xc0) != 0xc0) {
            domain_vec.push_back(std::string(it + 1, it + cnt + 1));
            it += cnt + 1;
        } else {
            compressed = true;
            u_short index = ((cnt & 0x3f) << 8) | it[1];
            auto new_it = pstart + index;
            domain_vec.push_back(decode_domain(pstart, pend, new_it));
            it += 2;
            break;
        }
    }
    if (!compressed) ++it;
    return string_join(domain_vec, ".");
}

dns::detail dns::ntoh(const detail &d, bool reverse)
{
    detail dt = d;
    ntoh_cvt(dt.id, !reverse, s);
    ntoh_cvt(dt.flags, !reverse, s);
    ntoh_cvt(dt.qrn, !reverse, s);
    ntoh_cvt(dt.rrn, !reverse, s);
    ntoh_cvt(dt.arn, !reverse, s);
    ntoh_cvt(dt.ern, !reverse, s);
    return dt;
}

dns::detail dns::hton(const detail &d) { return ntoh(d, true); }
