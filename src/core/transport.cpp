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
