#pragma once
#include "packet.h"
#include "pcap.h"
#include <chrono>
#define NT_TRY try {
#define NT_CATCH \
    }            \
    catch (const std::runtime_error &e) { LOG(ERROR) << e.what(); }

class transport
{
public:
    static pcap_t *open_adaptor(const adaptor &apt);

    static void setfilter(pcap_t *handle, const std::string &filter, const ip4 &mask);

    static bool recv(
        pcap_t *handle, std::function<bool(const packet &p)> callback, int timeout_ms = -1,
        const std::chrono::system_clock::time_point &start_tm = std::chrono::system_clock::now());

    static void send(pcap_t *handle, const packet &pac);

    static bool request(pcap_t *handle, const packet &req, packet &reply, int timeout_ms = -1,
                        bool do_send = true);

    static bool ip2mac(pcap_t *handle, const ip4 &ip, mac &mac_, bool use_cache = true,
                       int timeout_ms = 5000);
};
