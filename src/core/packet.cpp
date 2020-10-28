#include "packet.h"
#include "helper.h"
#include "ethernet.h"
#include "arp.h"

std::map<std::string, packet::decoder_type> packet::decoder_dict = {
    {Protocol_Type_Ethernet, &packet::decoder<ethernet>},
    {Protocol_Type_ARP, &packet::decoder<::arp>},
    {Protocol_Type_RARP, &packet::decoder<::arp>},
};

packet::packet(const u_char *const start, const u_char *const end)
{
    const u_char *pstart = start;
    std::string type = Protocol_Type_Ethernet;
    while (type != Protocol_Type_Void && pstart < end) {
        const u_char *pend;
        std::shared_ptr<protocol> prot;
        if (type == Protocol_Type_Ethernet) {
            prot = std::make_shared<ethernet>(pstart, pend);
        } else if (type == Protocol_Type_ARP || type == Protocol_Type_RARP) {
            prot = std::make_shared<::arp>(pstart, pend);
        } else {
            VLOG(1) << "unexpected protocol type after " << stack.back()->type() << ": " << type;
            break;
        }
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

packet packet::arp(const ip4 &dest)
{
    auto &apt = helper::get_adapter(dest);
    return arp(false, false, apt.mac_, apt.ip, mac::broadcast, dest);
}

packet packet::arp(bool reverse, bool reply, const mac &smac, const ip4 &sip, const mac &dmac,
                   const ip4 &dip)
{
    packet pkt;
    pkt.stack.push_back(std::make_shared<ethernet>(
        mac::broadcast, smac, reverse ? Protocol_Type_RARP : Protocol_Type_ARP));
    pkt.stack.push_back(std::make_shared<::arp>(reverse, reply, smac, sip, dmac, dip));
    return pkt;
}
