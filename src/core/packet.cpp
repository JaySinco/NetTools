#include "packet.h"
#include "ethernet.h"
#include "arp.h"
#include "ipv4.h"
#include "icmp.h"
#include "udp.h"
#include "dns.h"

std::map<std::string, packet::decoder> packet::decoder_dict = {
    {Protocol_Type_Ethernet, packet::decode<::ethernet>},
    {Protocol_Type_ARP, packet::decode<::arp>},
    {Protocol_Type_RARP, packet::decode<::arp>},
    {Protocol_Type_IPv4, packet::decode<::ipv4>},
    {Protocol_Type_ICMP, packet::decode<::icmp>},
    {Protocol_Type_UDP, packet::decode<::udp>},
    {Protocol_Type_DNS, packet::decode<::dns>},
};

std::string tv2s(const timeval &tv)
{
    tm local;
    time_t timestamp = tv.tv_sec;
    localtime_s(&local, &timestamp);
    char timestr[16] = {0};
    strftime(timestr, sizeof(timestr), "%H:%M:%S", &local);
    return fmt::format("{}.{:03d}", timestr, tv.tv_usec / 1000);
}

long operator-(const timeval &tv1, const timeval &tv2)
{
    long diff_sec = tv1.tv_sec - tv2.tv_sec;
    long diff_ms = (tv1.tv_usec - tv2.tv_usec) / 1000;
    return (diff_sec * 1000 + diff_ms);
}

packet::packet() { d.time = gettimeofday(); }

packet::packet(const u_char *const start, const u_char *const end, const timeval &tv)
{
    const u_char *pstart = start;
    std::string type = Protocol_Type_Ethernet;
    while (type != Protocol_Type_Void && pstart < end) {
        if (decoder_dict.count(type) <= 0) {
            VLOG(3) << "unimplemented protocol: " << d.layers.back()->type() << " -> " << type;
            break;
        }
        const u_char *pend = end;
        std::shared_ptr<protocol> prot =
            decoder_dict.at(type)(pstart, pend, d.layers.size() > 0 ? &*d.layers.back() : nullptr);
        if (pend > end) {
            throw std::runtime_error(fmt::format("exceed data boundary after {}", type));
        }
        d.layers.push_back(prot);
        pstart = pend;
        type = prot->succ_type();
    }
    d.time = tv;
}

timeval packet::gettimeofday()
{
    SYSTEMTIME st;
    GetLocalTime(&st);
    tm t = {0};
    t.tm_year = st.wYear - 1900;
    t.tm_mon = st.wMonth - 1;
    t.tm_mday = st.wDay;
    t.tm_hour = st.wHour;
    t.tm_min = st.wMinute;
    t.tm_sec = st.wSecond;
    t.tm_isdst = -1;
    time_t clock = mktime(&t);
    timeval tv;
    tv.tv_sec = clock;
    tv.tv_usec = st.wMilliseconds * 1000;
    return tv;
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
    j["time"] = tv2s(d.time);
    return j;
}

bool packet::link_to(const packet &rhs) const
{
    if (d.layers.size() != rhs.d.layers.size()) {
        return false;
    }
    if (d.time.tv_sec > rhs.d.time.tv_sec) {
        return false;
    }
    if (rhs.d.layers.size() > 2 && rhs.d.layers.at(2)->type() == Protocol_Type_ICMP) {
        auto &ch = dynamic_cast<const icmp &>(*rhs.d.layers.at(2));
        if (ch.icmp_type() == "error" && d.layers.size() > 1 &&
            d.layers.at(1)->type() == Protocol_Type_IPv4) {
            auto &ih = dynamic_cast<const ipv4 &>(*d.layers.at(1));
            if (ch.get_extra().eip == ih) {
                return true;
            }
        }
    }
    for (int i = 0; i < d.layers.size(); ++i) {
        if (!d.layers.at(i)->link_to(*rhs.d.layers.at(i))) {
            return false;
        }
    }
    return true;
}

const packet::detail &packet::get_detail() const { return d; }

void packet::set_time(const timeval &tv) { d.time = tv; }

bool packet::is_error() const
{
    return std::find_if(d.layers.cbegin(), d.layers.cend(),
                        [](const std::shared_ptr<protocol> &pt) {
                            if (pt->type() == Protocol_Type_ICMP) {
                                auto &ch = dynamic_cast<const icmp &>(*pt);
                                return ch.icmp_type() == "error";
                            }
                            return false;
                        }) != d.layers.cend();
}

packet packet::arp(const mac &smac, const ip4 &sip, const mac &dmac, const ip4 &dip, bool reply,
                   bool reverse)
{
    packet p;
    p.d.layers.push_back(std::make_shared<ethernet>(
        smac, mac::broadcast, reverse ? Protocol_Type_RARP : Protocol_Type_ARP));
    p.d.layers.push_back(std::make_shared<::arp>(smac, sip, dmac, dip, reply, reverse));
    return p;
}

packet packet::ping(const mac &smac, const ip4 &sip, const mac &dmac, const ip4 &dip, u_char ttl,
                    const std::string &echo, bool forbid_slice)
{
    packet p;
    p.d.layers.push_back(std::make_shared<ethernet>(smac, dmac, Protocol_Type_IPv4));
    p.d.layers.push_back(std::make_shared<ipv4>(sip, dip, ttl, Protocol_Type_ICMP, forbid_slice));
    p.d.layers.push_back(std::make_shared<icmp>(echo));
    return p;
}
