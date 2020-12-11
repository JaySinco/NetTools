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
    return "{}.{:03d}"_format(timestr, tv.tv_usec / 1000);
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
            throw std::runtime_error("exceed data boundary after {}"_format(type));
        }
        d.layers.push_back(prot);
        pstart = pend;
        type = prot->succ_type();
    }
    d.time = tv;
    d.owner = get_owner();
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
    j["owner"] = d.owner;
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

bool packet::has_type(const std::string &type) const
{
    return std::find_if(d.layers.cbegin(), d.layers.cend(),
                        [&](const std::shared_ptr<protocol> &pt) { return pt->type() == type; }) !=
           d.layers.cend();
}

std::string packet::get_owner() const
{
    using namespace std::chrono;
    static port_pid_table tb_udp, tb_tcp;
    static system_clock::time_point tm_udp, tm_tcp;

    port_pid_table *tb = nullptr;
    ip4 ip;
    u_short port = 0;
    auto now = system_clock::now();
    if (has_type(Protocol_Type_UDP)) {
        if (duration_cast<seconds>(now - tm_udp).count() > 3) {
            tb_udp = port_pid_table::udp();
            tm_udp = now;
        }
        tb = &tb_udp;
        const auto &ih = dynamic_cast<const ipv4 &>(*d.layers[1]);
        const auto &uh = dynamic_cast<const udp &>(*d.layers[2]);
        if (adaptor::is_native(ih.get_detail().sip)) {
            ip = ih.get_detail().sip;
            port = uh.get_detail().sport;
        } else if (adaptor::is_native(ih.get_detail().dip)) {
            ip = ih.get_detail().dip;
            port = uh.get_detail().dport;
        } else {
            return "";
        }

    } else if (has_type(Protocol_Type_TCP)) {
        if (duration_cast<seconds>(now - tm_tcp).count() > 3) {
            tb_tcp = port_pid_table::tcp();
            tm_tcp = now;
        }
        tb = &tb_tcp;
        return "";
    } else {
        return "";
    }

    auto key = std::make_pair(ip, port);
    if (tb->mapping.count(key) <= 0) {
        return "";
    }
    return pid_to_image(tb->mapping.at(key));
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
