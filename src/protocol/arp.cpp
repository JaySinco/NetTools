#include "arp.h"

arp::arp(const u_char *const start, const u_char *&end)
{
    d = *reinterpret_cast<const detail *>(start);
    end = start + sizeof(detail);
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
    j["hardware-address-length"] = d.hw_len;
    j["protocol-address-length"] = d.prot_len;
    u_short op = ntohs(d.op);
    j["operation"] = (op == 1 || op == 3)
                         ? "request"
                         : (op == 2 || op == 4) ? "reply" : Protocol_Type_Unimplemented(op);
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
               : (op == 3 || op == 4) ? Protocol_Type_RARP : Protocol_Type_Unimplemented(op);
}

std::string arp::succ_type() const { return Protocol_Type_Void; }
