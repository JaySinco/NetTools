#include "net.h"

int main(int argc, char* argv[])
{
    NT_TRY
    google::InitGoogleLogging(argv[0]);
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    FLAGS_logtostderr = 1;
    FLAGS_minloglevel = 0;

    if (argc < 2) {
        LOG(ERROR) << "empty target name, please iput ip or host name";
        return -1;
    }

    ip4_addr target_ip;
    std::string target_name = argv[1];
    if (ip4_addr::is_valid(target_name)) {
        target_ip = ip4_addr(target_name);
    }
    else {
        VLOG(1) << "invalid ip address, try interpret as host name";
        bool succ = false;
        target_ip = ip4_addr::from_hostname(target_name, succ);
        if (!succ) {
            LOG(ERROR) << "invalid ip or host name: " << target_name;
            return -1;
        }
    }

    std::cout << "target=" << target_ip << std::endl;
    adapter_info apt_info;
    pcap_t *adhandle = open_target_adaptor(PLACEHOLDER_IPv4_ADDR, false, apt_info);
    int ttl = 1;
    while (true) {
        _icmp_error_detail d_err = {0};
        int rtn = ping_with_ttl(adhandle, apt_info, target_ip, ttl, 10000, d_err);
        if (rtn == NTLS_TIMEOUT_ERROR) {
            std::cout << "timeout" << std::endl;
            break;
        }
        else if (rtn == NTLS_SUCC) {
            std::cout << "reach " << target_ip << std::endl;
            break;
        }
        else if (rtn == NTLS_FAILED) {
            std::cout << "route by " << d_err.h_aux.h.d.sia << std::endl;
        }
        else {
            std::cout << "error= " << rtn << std::endl;
            break;
        }
        ++ttl;
    }
    NT_CATCH
}