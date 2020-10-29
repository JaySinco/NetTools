#include "arp.h"

arp::arp(const u_char *const start, const u_char *&end)
{
    d = *reinterpret_cast<const detail *>(start);
    end = start + sizeof(detail);
}

arp::arp(const mac &smac, const ip4 &sip, const mac &dmac, const ip4 &dip, bool reply, bool reverse)
{
    u_short op = reverse ? (reply ? 4 : 3) : (reply ? 2 : 1);
    d.hw_type = htons(1);
    d.prot_type = htons(0x0800);
    d.hw_len = 6;
    d.prot_len = 4;
    d.op = htons(op);
    d.smac = smac;
    d.sip = sip;
    d.dmac = dmac;
    d.dip = dip;
}

arp::~arp() {}

void arp::to_bytes(std::vector<u_char> &bytes) const
{
    auto it = reinterpret_cast<const u_char *>(&d);
    bytes.insert(bytes.cbegin(), it, it + sizeof(detail));
}

json arp::to_json() const
{
    json j;
    j["type"] = type();
    j["hardware-type"] = ntohs(d.hw_type);
    j["protocol-type"] = ntohs(d.prot_type);
    j["hardware-addr-len"] = d.hw_len;
    j["protocol-addr-len"] = d.prot_len;
    u_short op = ntohs(d.op);
    j["operate"] = (op == 1 || op == 3) ? "request"
                                        : (op == 2 || op == 4) ? "reply" : Protocol_Type_Unknow(op);
    j["source-mac"] = d.smac.to_str();
    j["source-ip"] = d.sip.to_str();
    j["dest-mac"] = d.dmac.to_str();
    j["dest-ip"] = d.dip.to_str();
    return j;
}

std::string arp::type() const
{
    u_short op = ntohs(d.op);
    return (op == 1 || op == 2)
               ? Protocol_Type_ARP
               : (op == 3 || op == 4) ? Protocol_Type_RARP : Protocol_Type_Unknow(op);
}

std::string arp::succ_type() const { return Protocol_Type_Void; }

bool arp::link_to(const protocol &rhs) const
{
    if (type() == rhs.type()) {
        auto p = dynamic_cast<const arp &>(rhs);
        u_short lhs_op = ntohs(d.op);
        u_short rhs_op = ntohs(p.d.op);
        return (lhs_op == 1 || lhs_op == 3) && (rhs_op == 2 || rhs_op == 4) && (d.dip == p.d.sip);
    }
    return false;
}
