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

bool transport::wait(pcap_t *handle, std::function<bool(const packet &p)> callback, int timeout_ms,
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
        if (callback(packet(start, start + info->len))) {
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

bool transport::send_then_wait(pcap_t *handle, const packet &pac, packet &reply, int timeout_ms)
{
    auto start_tm = std::chrono::system_clock::now();
    send(handle, pac);
    return wait(
        handle,
        [&](const packet &p) {
            if (pac.link_to(p)) {
                reply = p;
                return true;
            }
            return false;
        },
        timeout_ms, start_tm);
}
