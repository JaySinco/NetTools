#include "net.h"

DEFINE_string(ip, "", "target ipv4 address, also used to choose adapter if --self not set");
DEFINE_string(self, "", "self ipv4 address, used to choose adapter");

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

    ip4_addr target_ip(FLAGS_ip);
    ip4_addr self_ip = target_ip;
    if (FLAGS_self.size() > 0) {
        self_ip = ip4_addr(FLAGS_self);
    }
    adapter_info apt_info;
    pcap_t *adhandle = open_target_adaptor(self_ip, false, apt_info);
    if (!is_reachable(adhandle, apt_info, target_ip, 5000)) {
        std::cout << target_ip << " is not online!" << std::endl;
        return -1;
    }
    std::cout << target_ip << " is online!" << std::endl;
    NT_CATCH
}