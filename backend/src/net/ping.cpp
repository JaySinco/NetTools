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

    std::cout << "\nPing " << ip_desc.str() << ":" << std::endl;
    adapter_info apt_info;
    pcap_t *adhandle = open_target_adaptor(PLACEHOLDER_IPv4_ADDR, false, apt_info);
    constexpr int total_cnt = 4;
    int recv_cnt = 0;
    long min_cost = 999999;
    long max_cost = -1;
    long sum_cost = 0;
    for (int i = 0; i < total_cnt; ++i) {
        long cost_ms;
        ip_header ih_recv;
        _icmp_error_detail d_err;
        int rtn = ping(adhandle, apt_info, target_ip, 128, 3000, cost_ms, ih_recv, d_err);
        std::cout << "Reply from " << target_ip << ": ";
        if (rtn == NTLS_SUCC) {
            ++recv_cnt;
            min_cost = min(min_cost, cost_ms);
            max_cost = max(max_cost, cost_ms);
            sum_cost += cost_ms;
            std::cout << "time=" << cost_ms << "ms"
                      << " ttl=" << static_cast<int>(ih_recv.d.ttl) << std::endl;
        } else {
            std::cout << "timeout" << std::endl;
        }
    }
    std::cout << "\nStatistical information:" << std::endl;
    std::cout << "    packets: sent=" << total_cnt << ", recv=" << recv_cnt
              << ", lost=" << (total_cnt - recv_cnt) << " ("
              << int(float(total_cnt - recv_cnt) / total_cnt * 100) << "% lost)\n";
    if (sum_cost > 0) {
        std::cout << "Estimated time of round trip:" << std::endl;
        std::cout << "    min=" << min_cost << "ms, max=" << max_cost
                  << "ms, avg=" << (sum_cost) / total_cnt << "ms\n";
    }
    NT_CATCH
}