#include "net.h"
#include <iphlpapi.h>
#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <mutex>

pcap_t *net::open_adaptor(const adapter &apt)
{
    pcap_t *handle;
    char errbuf[PCAP_ERRBUF_SIZE];
    if (!(handle =
              pcap_open(apt.name.c_str(), 65536, PCAP_OPENFLAG_PROMISCUOUS, 1000, NULL, errbuf))) {
        throw std::runtime_error(fmt::format("failed to open adapter: {}", apt.name));
    }
    return handle;
}

pcap_t *net::open_adaptor(const ip4 &ip, adapter &apt)
{
    if (ip != ip4::placeholder) {
        auto it = std::find_if(all_adapters().begin(), all_adapters().end(),
                               [&](const adapter &dev) { return dev.ip.is_local(ip, dev.mask); });
        if (it == all_adapters().end()) {
            throw std::runtime_error(fmt::format("no local adapter match {}", ip.to_str()));
        }
        apt = *it;
    }
    apt = all_adapters().front();
    return open_adaptor(apt);
}

const std::vector<adapter> &net::all_adapters()
{
    static std::once_flag flag;
    static std::vector<adapter> adapters;
    std::call_once(flag, [&] {
        u_long buflen = sizeof(IP_ADAPTER_INFO);
        auto plist = reinterpret_cast<IP_ADAPTER_INFO *>(malloc(sizeof(IP_ADAPTER_INFO)));
        std::shared_ptr<void> plist_guard(nullptr, [=](void *) { free(plist); });
        if (GetAdaptersInfo(plist, &buflen) == ERROR_BUFFER_OVERFLOW) {
            plist = reinterpret_cast<IP_ADAPTER_INFO *>(malloc(buflen));
            if (GetAdaptersInfo(plist, &buflen) != NO_ERROR) {
                throw std::runtime_error("failed to get adapters info");
            }
        }
        PIP_ADAPTER_INFO pinfo = plist;
        while (pinfo) {
            adapter apt;
            ip4 ip(pinfo->IpAddressList.IpAddress.String);
            ip4 mask(pinfo->IpAddressList.IpMask.String);
            ip4 gateway(pinfo->GatewayList.IpAddress.String);
            if (gateway != ip4::placeholder && mask != ip4::placeholder) {
                apt.name = std::string("\\Device\\NPF_") + pinfo->AdapterName;
                apt.desc = pinfo->Description;
                apt.ip = ip;
                apt.mask = mask;
                apt.gateway = gateway;
                if (pinfo->AddressLength != sizeof(mac)) {
                    LOG(WARNING) << "wrong mac length: " << pinfo->AddressLength;
                } else {
                    auto c = reinterpret_cast<u_char *>(&apt.mac);
                    for (unsigned i = 0; i < pinfo->AddressLength; ++i) {
                        c[i] = pinfo->Address[i];
                    }
                }
                VLOG(1) << "find suitable adapter: " << apt.to_json().dump(3);
                adapters.push_back(apt);
            }
            pinfo = pinfo->Next;
        }
        if (adapters.size() <= 0) {
            throw std::runtime_error("failed to find any suitable adapter");
        }
    });
    return adapters;
}
