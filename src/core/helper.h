#pragma once
#include "type.h"
#include "pcap.h"
#define NT_TRY try {
#define NT_CATCH \
    }            \
    catch (const std::runtime_error &e) { LOG(ERROR) << e.what(); }

class helper
{
public:
    static pcap_t *open_adaptor(const adapter &apt);
    static const adapter &get_adapter(const ip4 &hint = ip4::zeros);
    static const std::vector<adapter> &all_adapters();
};
