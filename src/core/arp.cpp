#include "arp.h"

arp::arp(const u_char *const start, const u_char *&end)
{
    d = convert_detail(true, *reinterpret_cast<const detail *>(start));
    end = start + sizeof(detail);
}

arp::arp(const mac &smac, const ip4 &sip, const mac &dmac, const ip4 &dip, bool reply, bool reverse)
{
    d.hw_type = 1;
    d.prot_type = 0x0800;
    d.hw_len = 6;
    d.prot_len = 4;
    d.op = reverse ? (reply ? 4 : 3) : (reply ? 2 : 1);
    d.smac = smac;
    d.sip = sip;
    d.dmac = dmac;
    d.dip = dip;
}

arp::~arp() {}

void arp::to_bytes(std::vector<u_char> &bytes) const
{
    auto dt = convert_detail(false, d);
    auto it = reinterpret_cast<const u_char *>(&dt);
    bytes.insert(bytes.cbegin(), it, it + sizeof(detail));
}

json arp::to_json() const
{
    json j;
    j["type"] = type();
    j["hardware-type"] = d.hw_type;
    j["protocol-type"] = d.prot_type;
    j["hardware-addr-len"] = d.hw_len;
    j["protocol-addr-len"] = d.prot_len;
    j["operate"] = (d.op == 1 || d.op == 3)
                       ? "request"
                       : (d.op == 2 || d.op == 4) ? "reply" : Protocol_Type_Unknow(d.op);
    j["source-mac"] = d.smac.to_str();
    j["source-ip"] = d.sip.to_str();
    j["dest-mac"] = d.dmac.to_str();
    j["dest-ip"] = d.dip.to_str();
    return j;
}

std::string arp::type() const
{
    return (d.op == 1 || d.op == 2)
               ? Protocol_Type_ARP
               : (d.op == 3 || d.op == 4) ? Protocol_Type_RARP : Protocol_Type_Unknow(d.op);
}

std::string arp::succ_type() const { return Protocol_Type_Void; }

bool arp::link_to(const protocol &rhs) const
{
    if (type() == rhs.type()) {
        auto p = dynamic_cast<const arp &>(rhs);
        return (d.op == 1 || d.op == 3) && (p.d.op == 2 || p.d.op == 4) && (d.dip == p.d.sip);
    }
    return false;
}

const arp::detail &arp::get_detail() const { return d; }

arp::detail arp::convert_detail(bool ntoh, const detail &d)
{
    detail dt = d;
    NET_CVT(dt.hw_type, ntoh, s);
    NET_CVT(dt.prot_type, ntoh, s);
    NET_CVT(dt.op, ntoh, s);
    return dt;
}
