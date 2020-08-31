#include "net.h"

DEFINE_string(ip, "", "ipv4 address used to choose adapter");

int main(int argc, char* argv[])
{
    NT_TRY
    google::InitGoogleLogging(argv[0]);
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    FLAGS_logtostderr = 1;
    FLAGS_minloglevel = 0;

    if (FLAGS_ip.size() <= 0) {
        LOG(ERROR) << "empty ipv4 address, please set --ip";
        return -1;
    }

    ip4_addr input_ip(FLAGS_ip);
    adapter_info apt_info;
    pcap_t *adhandle = open_target_adaptor(input_ip, false, apt_info);
    ip4_addr netmask;
    if (!query_mask(adhandle, apt_info, netmask, 5000)) {
        LOG(ERROR) << "failed to query netmask for " << apt_info.ip;
        return -1;
    }
    LOG(INFO) << "netmask queried for " << apt_info.ip << " is " << netmask; 
    NT_CATCH
}