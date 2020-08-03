#include "net.h"

DEFINE_string(p, "", "ipv4 address");
// DEFINE_string(m, "", "ethernet physical address");
// DEFINE_bool(a, false, "attack network by faking arp reply");

int main(int argc, char* argv[])
{
    google::InitGoogleLogging(argv[0]);
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    FLAGS_logtostderr = 1;
    FLAGS_minloglevel = 0;

    if (FLAGS_p.size() <= 0) {
        LOG(ERROR) << "empty ipv4 address";
        return -1;
    }
    ip4_addr input_ip;
    if (!ip4_from_string(FLAGS_p, input_ip)) {
        LOG(ERROR) << "invalid ipv4 address: " << FLAGS_p;
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

    adapter_info apt_info;
    if (!get_adapter_info_by_ip4(local_ip, apt_info)) {
        LOG(ERROR) << "failed to get more adapter info";
        return -1;
    }

    pcap_t *adhandle;
    if (!(adhandle= pcap_open(devname.c_str(), 65536, PCAP_OPENFLAG_PROMISCUOUS, 1000, NULL, errbuf)))
    {
        LOG(ERROR) << "failed to open the adapter";
        return -1;
    }

    u_char packet[sizeof(ethernet_header) + sizeof(eth_ip4_arp)] = { 0 };
    auto eh = reinterpret_cast<ethernet_header*>(packet);
    eh->dea = BROADCAST_ETH_ADDR;
    eh->sea = apt_info.mac;
    eh->eth_type = htons(ETHERNET_TYPE_ARP);
    auto ah = reinterpret_cast<eth_ip4_arp*>(packet + sizeof(ethernet_header));
    ah->hw_type = htons(ARP_HARDWARE_TYPE_ETHERNET);
    ah->proto = htons(ARP_HARDWARE_PROTO_IP);
    ah->hw_len = ETHERNET_ADDRESS_LEN;
    ah->proto_len = IPV4_ADDRESS_LEN;
    ah->op = htons(ARP_REQUEST_OP);
    ah->sea = apt_info.mac;
    ah->sia = local_ip;
    ah->dia = input_ip;
    if (pcap_sendpacket(adhandle, packet, sizeof(packet)/sizeof(packet[0])) != 0)
    {
        LOG(ERROR) << "failed to send packet: " << pcap_geterr(adhandle);
        return -1;
    }

    int res;
    pcap_pkthdr *header;
    const u_char *pkt_data;
    while((res = pcap_next_ex(adhandle, &header, &pkt_data)) >= 0)
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
        LOG(ERROR) << "failed to read packets: " << pcap_geterr(adhandle);
        return -1;
    }
}