#include "net.h"

DEFINE_string(ip, "", "target ipv4 address");

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

    adapter_info apt_info;
    pcap_t *adhandle = open_target_adaptor(PLACEHOLDER_IPv4_ADDR, false, apt_info);
    ip4_addr target_ip(FLAGS_ip);
    if (!is_reachable(adhandle, apt_info, target_ip, 5000)) {
        std::cout << target_ip << " is not online!" << std::endl;
        return -1;
    }
    std::cout << target_ip << " is online!" << std::endl;
    NT_CATCH
}