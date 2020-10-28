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
            VLOG(1) << "unimplemented protocol: " << layers.back()->type() << " -> " << type;
            break;
        }
        const u_char *pend;
        std::shared_ptr<protocol> prot = decoder_dict.at(type)(pstart, pend);
        if (pend > end) {
            throw std::runtime_error(fmt::format("exceed data boundary after {}", type));
        }
        layers.push_back(prot);
        pstart = pend;
        type = prot->succ_type();
    }
}

void packet::to_bytes(std::vector<u_char> &bytes) const
{
    for (auto it = layers.crbegin(); it != layers.crend(); ++it) {
        (*it)->to_bytes(bytes);
    }
}

json packet::to_json() const
{
    json ar = json::array();
    for (auto it = layers.cbegin(); it != layers.cend(); ++it) {
        ar.push_back((*it)->to_json());
    }
    json j;
    j["layers"] = ar;
    if (recv_tm.tm_year != 0) {
        char timestr[16] = {0};
        strftime(timestr, sizeof(timestr), "%H:%M:%S", &recv_tm);
        j["time"] = fmt::format("{}.{}", timestr, recv_ms);
    }
    return j;
}

bool packet::link_to(const packet &rhs) const
{
    if (layers.size() != rhs.layers.size()) {
        return false;
    }
    for (int i = 0; i < layers.size(); ++i) {
        if (!layers.at(i)->link_to(*rhs.layers.at(i))) {
            return false;
        }
    }
    return true;
}

void packet::received_at(long sec, long ms)
{
    time_t local = sec;
    localtime_s(&recv_tm, &local);
    recv_ms = ms;
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
    p.layers.push_back(std::make_shared<ethernet>(
        mac::broadcast, smac, reverse ? Protocol_Type_RARP : Protocol_Type_ARP));
    p.layers.push_back(std::make_shared<::arp>(reverse, reply, smac, sip, dmac, dip));
    return p;
}
