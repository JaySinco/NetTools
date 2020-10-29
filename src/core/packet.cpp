#include "packet.h"
#include "ethernet.h"
#include "arp.h"

std::map<std::string, packet::decoder> packet::decoder_dict = {
    {Protocol_Type_Ethernet, packet::decode<ethernet>},
    {Protocol_Type_ARP, packet::decode<::arp>},
    {Protocol_Type_RARP, packet::decode<::arp>},
};

packet::packet(const u_char *const start, const u_char *const end, long recv_sec, long ms)
{
    const u_char *pstart = start;
    std::string type = Protocol_Type_Ethernet;
    while (type != Protocol_Type_Void && pstart < end) {
        if (decoder_dict.count(type) <= 0) {
            VLOG(3) << "unimplemented protocol: " << d.layers.back()->type() << " -> " << type;
            break;
        }
        const u_char *pend = end;
        std::shared_ptr<protocol> prot = decoder_dict.at(type)(pstart, pend);
        if (pend > end) {
            throw std::runtime_error(fmt::format("exceed data boundary after {}", type));
        }
        d.layers.push_back(prot);
        pstart = pend;
        type = prot->succ_type();
    }
    if (recv_sec > 0) {
        time_t local = recv_sec;
        localtime_s(&d.recv_tm, &local);
        d.recv_ms = ms;
    }
}

void packet::to_bytes(std::vector<u_char> &bytes) const
{
    for (auto it = d.layers.crbegin(); it != d.layers.crend(); ++it) {
        (*it)->to_bytes(bytes);
    }
}

json packet::to_json() const
{
    json ar = json::array();
    for (auto it = d.layers.cbegin(); it != d.layers.cend(); ++it) {
        ar.push_back((*it)->to_json());
    }
    json j;
    j["layers"] = ar;
    if (d.recv_tm.tm_year != 0) {
        char timestr[16] = {0};
        strftime(timestr, sizeof(timestr), "%H:%M:%S", &d.recv_tm);
        j["time"] = fmt::format("{}.{}", timestr, d.recv_ms);
    }
    return j;
}

bool packet::link_to(const packet &rhs) const
{
    if (d.layers.size() != rhs.d.layers.size()) {
        return false;
    }
    for (int i = 0; i < d.layers.size(); ++i) {
        if (!d.layers.at(i)->link_to(*rhs.d.layers.at(i))) {
            return false;
        }
    }
    return true;
}

const packet::detail &packet::get_detail() const { return d; }

packet packet::arp(const ip4 &dest)
{
    auto &apt = adaptor::fit(dest);
    return arp(apt.mac_, apt.ip, mac::zeros, dest);
}

packet packet::arp(const mac &smac, const ip4 &sip, const mac &dmac, const ip4 &dip, bool reply,
                   bool reverse)
{
    packet p;
    p.d.layers.push_back(std::make_shared<ethernet>(
        mac::broadcast, smac, reverse ? Protocol_Type_RARP : Protocol_Type_ARP));
    p.d.layers.push_back(std::make_shared<::arp>(smac, sip, dmac, dip, reply, reverse));
    return p;
}
