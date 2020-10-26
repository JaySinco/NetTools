#pragma once
#include "type.h"
#include "pcap.h"
#define NT_TRY try {
#define NT_CATCH \
    }            \
    catch (const std::runtime_error &e) { LOG(ERROR) << e.what(); }

class net
{
public:
    static const std::vector<adapter> &all_adapters();
    static pcap_t *open_adaptor(const adapter &apt);
    static pcap_t *open_adaptor(const ip4 &ip, adapter &apt);
};
