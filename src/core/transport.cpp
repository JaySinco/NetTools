#include "transport.h"
#include "arp.h"
#include <atomic>
#include <thread>

pcap_t *transport::open_adaptor(const adaptor &apt)
{
    pcap_t *handle;
    char *errbuf = new char[PCAP_ERRBUF_SIZE];
    if (!(handle =
              pcap_open(apt.name.c_str(), 65536, PCAP_OPENFLAG_PROMISCUOUS, 1000, NULL, errbuf))) {
        throw std::runtime_error(fmt::format("failed to open adapter: {}", apt.name));
    }
    return handle;
}

void transport::set_filter(pcap_t *handle, const std::string &filter, const ip4 &mask)
{
    bpf_program fcode;
    if (pcap_compile(handle, &fcode, filter.c_str(), 1, static_cast<u_int>(mask)) < 0) {
        throw std::runtime_error(
            fmt::format("failed to compile pcap filter: {}, please refer to "
                        "https://nmap.org/npcap/guide/wpcap/pcap-filter.html",
                        filter));
    }
    if (pcap_setfilter(handle, &fcode) < 0) {
        throw std::runtime_error(fmt::format("failed to set pcap filter: {}", filter));
    }
}

bool transport::recv(pcap_t *handle, std::function<bool(const packet &p)> callback, int timeout_ms,
                     const std::chrono::system_clock::time_point &start_tm)
{
    int res;
    pcap_pkthdr *info;
    const u_char *start;
    while ((res = pcap_next_ex(handle, &info, &start)) >= 0) {
        if (timeout_ms > 0) {
            auto now = std::chrono::system_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_tm);
            if (duration.count() >= timeout_ms) {
                return false;
            }
        }
        if (res == 0) {
            continue;  // timeout elapsed
        }
        if (callback(packet(start, start + info->len, info->ts.tv_sec, info->ts.tv_usec))) {
            return true;
        }
    }
    if (res == -1) {
        throw std::runtime_error(fmt::format("failed to read packets: {}", pcap_geterr(handle)));
    }
    throw std::runtime_error("stop reading packets due to unexpected error");
}

void transport::send(pcap_t *handle, const packet &pac)
{
    std::vector<u_char> bytes;
    pac.to_bytes(bytes);
    if (pcap_sendpacket(handle, bytes.data(), bytes.size()) != 0) {
        throw std::runtime_error(fmt::format("failed to send packet: {}", pcap_geterr(handle)));
    }
}

bool transport::request(pcap_t *handle, const packet &req, packet &reply, int timeout_ms,
                        bool do_send)
{
    auto start_tm = std::chrono::system_clock::now();
    if (do_send) {
        send(handle, req);
    }
    return recv(
        handle,
        [&](const packet &p) {
            if (req.link_to(p)) {
                reply = p;
                return true;
            }
            return false;
        },
        timeout_ms, start_tm);
}

bool transport::ip2mac(pcap_t *handle, const ip4 &ip, mac &mac_, bool use_cache, int timeout_ms)
{
    static std::map<ip4, std::pair<mac, std::chrono::time_point<std::chrono::system_clock>>> cached;
    auto start_tm = std::chrono::system_clock::now();
    if (use_cache && cached.count(ip) > 0) {
        auto passed_sec =
            std::chrono::duration_cast<std::chrono::seconds>(start_tm - cached[ip].second);
        if (passed_sec.count() < 30) {
            VLOG(3) << "use cached mac for " << ip.to_str();
            mac_ = cached[ip].first;
            return true;
        } else {
            VLOG(3) << "cached mac for " << ip.to_str() << " expired, send arp to update";
        }
    }
    std::atomic<bool> over = false;
    packet pac = packet::arp(ip);
    std::thread send_loop([&] {
        while (!over) {
            send(handle, pac);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    });
    packet reply;
    bool ret = transport::request(handle, pac, reply, timeout_ms, false);
    if (ret) {
        const protocol &prot = *reply.get_detail().layers.back();
        mac_ = dynamic_cast<const arp &>(prot).get_detail().smac;
        cached[ip] = std::make_pair(mac_, std::chrono::system_clock::now());
    }
    over = true;
    send_loop.join();
    return ret;
}
