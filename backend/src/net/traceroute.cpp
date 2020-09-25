#include <iomanip>
#include "net.h"

int main(int argc, char *argv[])
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
    std::ostringstream ip_desc;
    if (ip4_addr::is_valid(target_name)) {
        target_ip = ip4_addr(target_name);
        ip_desc << target_ip;
    } else {
        VLOG(1) << "invalid ip address, try interpret as host name";
        bool succ = false;
        target_ip = ip4_addr::from_hostname(target_name, succ);
        if (!succ) {
            LOG(ERROR) << "invalid ip or host name: " << target_name;
            return -1;
        }
        ip_desc << target_name << " [" << target_ip << "]";
    }

    std::cout << "\nRoute traced to " << ip_desc.str() << "\n" << std::endl;
    adapter_info apt_info;
    pcap_t *adhandle = open_target_adaptor(PLACEHOLDER_IPv4_ADDR, false, apt_info);
    int ttl = 0;
    constexpr int epoch_cnt = 3;
    while (true) {
        ++ttl;
        ip4_addr route_ip;
        int timeout_cnt = 0;
        std::cout << std::setw(2) << ttl << " " << std::flush;
        for (int i = 0; i < epoch_cnt; ++i) {
            std::cout << std::setw(6);
            long cost_ms;
            ip_header ih_recv;
            _icmp_error_detail d_err;
            int rtn = ping(adhandle, apt_info, target_ip, ttl, 3000, cost_ms, ih_recv, d_err);
            if (rtn == NTLS_TIMEOUT_ERROR) {
                ++timeout_cnt;
                std::cout << "       *" << std::flush;
            } else if (rtn == NTLS_SUCC) {
                route_ip = target_ip;
                std::cout << cost_ms << "ms" << std::flush;
            } else if (rtn == NTLS_RECV_ERROR_ICMP) {
                route_ip = d_err.h_aux.h.d.sia;
                std::cout << cost_ms << "ms" << std::flush;
            }
        }
        if (timeout_cnt >= epoch_cnt) {
            std::cout << "  -- timeout --" << std::endl;
        } else {
            std::cout << "  " << route_ip << std::endl;
            if (route_ip == target_ip) {
                std::cout << "\nTracking is complete." << std::endl;
                break;
            }
        }
    }
    NT_CATCH
}