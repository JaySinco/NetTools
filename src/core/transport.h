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

    static bool wait(
        pcap_t *handle, std::function<bool(const packet &p)> callback, int timeout_ms = -1,
        const std::chrono::system_clock::time_point &start_tm = std::chrono::system_clock::now());

    static void send(pcap_t *handle, const packet &pac);
    static bool send_then_wait(pcap_t *handle, const packet &pac, packet &reply,
                               int timeout_ms = -1);
};
