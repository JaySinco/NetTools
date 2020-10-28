#include "transport.h"

pcap_t *transport::open_adaptor(const adaptor &apt)
{
    pcap_t *handle;
    char errbuf[PCAP_ERRBUF_SIZE];
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

bool transport::send_and_recv(pcap_t *handle, const packet &req, packet &reply, int timeout_ms)
{
    auto start_tm = std::chrono::system_clock::now();
    send(handle, req);
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
