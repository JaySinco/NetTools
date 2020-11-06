#include "core/transport.h"
#include "core/ipv4.h"
#include <iostream>
#include <iomanip>

int main(int argc, char *argv[])
{
    NT_TRY
    google::InitGoogleLogging(argv[0]);
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    FLAGS_logtostderr = 1;
    FLAGS_minloglevel = 0;

    if (argc < 2) {
        LOG(ERROR) << "empty target name, please input ip or host name";
        return -1;
    }

    ip4 target_ip;
    std::string target_name = argv[1];
    std::ostringstream ip_desc;
    if (ip4::from_dotted_dec(target_name, &target_ip)) {
        ip_desc << target_ip.to_str();
    } else {
        if (!ip4::from_domain(target_name, &target_ip)) {
            LOG(ERROR) << "invalid ip or host name: " << target_name;
            return -1;
        }
        ip_desc << target_name << " [" << target_ip.to_str() << "]";
    }

    std::cout << "\nRoute traced to " << ip_desc.str() << "\n" << std::endl;
    auto &apt = adaptor::fit(ip4::zeros);
    pcap_t *handle = transport::open_adaptor(apt);
    int ttl = 0;
    constexpr int epoch_cnt = 3;
    while (true) {
        ++ttl;
        ip4 router_ip;
        int timeout_cnt = 0;
        std::cout << std::setw(2) << ttl << " " << std::flush;
        for (int i = 0; i < epoch_cnt; ++i) {
            std::cout << std::setw(6);
            packet reply;
            long cost_ms;
            if (transport::ping(handle, apt, target_ip, reply, cost_ms, ttl, "greatjaysinco")) {
                if (reply.is_error()) {
                    auto &ih = dynamic_cast<const ipv4 &>(*reply.get_detail().layers.at(1));
                    router_ip = ih.get_detail().sip;
                    std::cout << cost_ms << "ms" << std::flush;
                } else {
                    router_ip = target_ip;
                    std::cout << cost_ms << "ms" << std::flush;
                }
            } else {
                ++timeout_cnt;
                std::cout << "       *" << std::flush;
            }
        }
        if (timeout_cnt >= epoch_cnt) {
            std::cout << "  -- timeout --" << std::endl;
        } else {
            std::cout << "  " << router_ip.to_str() << std::endl;
            if (router_ip == target_ip) {
                std::cout << "\nTracking is complete." << std::endl;
                break;
            }
        }
    }
    NT_CATCH
}