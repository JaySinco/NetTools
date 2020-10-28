#include "packet.h"
#include "ethernet.h"
#include "arp.h"

std::map<std::string, packet::decoder> packet::decoder_dict = {
    {Protocol_Type_Ethernet, packet::decode<ethernet>},
    {Protocol_Type_ARP, packet::decode<::arp>},
    {Protocol_Type_RARP, packet::decode<::arp>},
};

packet::packet(const u_char *const start, const u_char *const end)
{
    const u_char *pstart = start;
    std::string type = Protocol_Type_Ethernet;
    while (type != Protocol_Type_Void && pstart < end) {
        if (decoder_dict.count(type) <= 0) {
            VLOG(1) << "unimplemented protocol: " << stack.back()->type() << " -> " << type;
            break;
        }
        const u_char *pend;
        std::shared_ptr<protocol> prot = decoder_dict.at(type)(pstart, pend);
        if (pend > end) {
            throw std::runtime_error(fmt::format("exceed data boundary after {}", type));
        }
        stack.push_back(prot);
        pstart = pend;
        type = prot->succ_type();
    }
}

void packet::to_bytes(std::vector<u_char> &bytes) const
{
    for (auto it = stack.crbegin(); it != stack.crend(); ++it) {
        (*it)->to_bytes(bytes);
    }
}

json packet::to_json() const
{
    json j;
    for (auto it = stack.cbegin(); it != stack.cend(); ++it) {
        j.push_back((*it)->to_json());
    }
    return j;
}

bool packet::link_to(const packet &rhs) const
{
    if (stack.size() != rhs.stack.size()) {
        return false;
    }
    for (int i = 0; i < stack.size(); ++i) {
        if (!stack.at(i)->link_to(*rhs.stack.at(i))) {
            return false;
        }
    }
    return true;
}

packet packet::arp(const ip4 &dest)
{
    auto &apt = adaptor::fit(dest);
    return arp(false, false, apt.mac_, apt.ip, mac::broadcast, dest);
}

packet packet::arp(bool reverse, bool reply, const mac &smac, const ip4 &sip, const mac &dmac,
                   const ip4 &dip)
{
    packet p;
    p.stack.push_back(std::make_shared<ethernet>(mac::broadcast, smac,
                                                 reverse ? Protocol_Type_RARP : Protocol_Type_ARP));
    p.stack.push_back(std::make_shared<::arp>(reverse, reply, smac, sip, dmac, dip));
    return p;
}
