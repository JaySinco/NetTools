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
            rr.dlen = ntohs(*reinterpret_cast<const u_short *>(it));
            it += sizeof(u_short);
            if (rr.type == 5) {  // CNAME
                rr.data = decode_domain(start, end, it);
            } else {
                rr.data = std::string(it, it + rr.dlen);
                it += rr.dlen;
            }
            vec.push_back(rr);
        }
    };
    parse_res(extra.reply, d.rrn);
    parse_res(extra.auth, d.arn);
    parse_res(extra.extra, d.ern);
}

dns::dns(const std::string &query_domain)
{
    d.id = rand_ushort();
    d.flags |= 0 << 15;          // qr
    d.flags |= (0 & 0xf) << 11;  // opcode
    d.flags |= 0 << 10;          // authoritative answer
    d.flags |= 1 << 9;           // truncated
    d.flags |= 1 << 8;           // recursion desired
    d.flags |= 0 << 7;           // recursion available
    d.flags |= (0 & 0xf);        // rcode
    d.qrn = 1;
    query_detail qr;
    qr.domain = query_domain;
    qr.type = 1;  // A
    qr.cls = 1;   // internet address
    extra.query.push_back(qr);
}

void dns::to_bytes(std::vector<u_char> &bytes) const
{
    auto dt = hton(d);
    auto it = reinterpret_cast<const u_char *>(&dt);
    bytes.insert(bytes.cbegin(), it, it + sizeof(detail));
    for (const auto &qr : extra.query) {
        std::string name = encode_domain(qr.domain);
        bytes.insert(bytes.end(), name.cbegin(), name.cend());
        u_short type = htons(qr.type);
        auto pt = reinterpret_cast<u_char *>(&type);
        bytes.insert(bytes.end(), pt, pt + sizeof(u_short));
        u_short cls = htons(qr.cls);
        auto pc = reinterpret_cast<u_char *>(&cls);
        bytes.insert(bytes.end(), pc, pc + sizeof(u_short));
    }
    auto encode_res = [&](const std::vector<res_detail> &rd) {
        for (const auto &rr : rd) {
            std::string name = encode_domain(rr.domain);
            bytes.insert(bytes.end(), name.cbegin(), name.cend());
            u_short type = htons(rr.type);
            auto pt = reinterpret_cast<u_char *>(&type);
            bytes.insert(bytes.end(), pt, pt + sizeof(u_short));
            u_short cls = htons(rr.cls);
            auto pc = reinterpret_cast<u_char *>(&cls);
            bytes.insert(bytes.end(), pc, pc + sizeof(u_short));
            u_int ttl = htonl(rr.ttl);
            auto pl = reinterpret_cast<u_char *>(&ttl);
            bytes.insert(bytes.end(), pl, pl + sizeof(u_int));
            u_short dlen = htons(rr.dlen);
            auto pd = reinterpret_cast<u_char *>(&dlen);
            bytes.insert(bytes.end(), pd, pd + sizeof(u_short));
            bytes.insert(bytes.end(), rr.data.data(), rr.data.data() + rr.data.size());
        }
    };
    encode_res(extra.reply);
    encode_res(extra.auth);
    encode_res(extra.extra);
}

json dns::to_json() const
{
    json j;
    j["type"] = type();
    j["id"] = d.id;
    j["dns-type"] = d.flags & 0x8000 ? "reply" : "query";
    j["opcode"] = (d.flags >> 11) & 0xf;
    j["authoritative-answer"] = d.flags & 0x400 ? true : false;
    j["truncated"] = d.flags & 0x200 ? true : false;
    j["recursion-desired"] = d.flags & 0x100 ? true : false;
    j["recursion-available"] = d.flags & 0x80 ? true : false;
    j["rcode"] = d.flags & 0xf;
    j["query-count"] = d.qrn;
    j["reply-count"] = d.rrn;
    j["author-count"] = d.arn;
    j["extra-count"] = d.ern;
    if (d.qrn > 0) {
        json query;
        for (const auto &qr : extra.query) {
            json r;
            r["domain"] = qr.domain;
            r["query-type"] = qr.type;
            r["query-class"] = qr.cls;
            query.push_back(r);
        }
        j["query"] = query;
    }
    auto jsonify_res = [](const std::vector<res_detail> &rd) -> json {
        json res;
        for (const auto &rr : rd) {
            json r;
            r["domain"] = rr.domain;
            r["query-type"] = rr.type;
            r["query-class"] = rr.cls;
            r["ttl"] = rr.ttl;
            r["data-size"] = rr.dlen;
            if (rr.type == 1) {  // A
                r["data"] = reinterpret_cast<const ip4 *>(rr.data.data())->to_str();
            } else if (rr.type == 5) {  // CNAME
                r["data"] = rr.data;
            }
            res.push_back(r);
        }
        return res;
    };
    if (d.rrn > 0) j["reply"] = jsonify_res(extra.reply);
    if (d.arn > 0) j["author"] = jsonify_res(extra.auth);
    if (d.ern > 0) j["extra"] = jsonify_res(extra.extra);
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
            throw std::runtime_error("segment of domain exceed 63: {}"_format(s));
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
