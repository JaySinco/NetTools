#pragma once
#include "type.h"
#include "pcap.h"
#define NT_TRY try {
#define NT_CATCH \
    }            \
    catch (const std::runtime_error &e) { LOG(ERROR) << e.what(); }

class transport
{
public:
    static pcap_t *open_adaptor(const adaptor &apt);
};
