#include "net.h"

DEFINE_int32(i, -1, "adapter index to capture");
DEFINE_string(f, "", "capture filter applied to adapter");

int main(int argc, char* argv[])
{
    google::InitGoogleLogging(argv[0]);
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    FLAGS_logtostderr = 1;
    FLAGS_minloglevel = 0;

    if (FLAGS_i < 0) {
        LOG(ERROR) << "invalid adapter index " << FLAGS_i;
        return -1;
    }
    std::string devname;
    u_int devmask = 0xffffff;
    pcap_if_t *alldevs;
    char errbuf[PCAP_ERRBUF_SIZE];
    if (pcap_findalldevs(&alldevs, errbuf) == -1)
    {
        LOG(ERROR) << "failed to find all device: " << errbuf;
        return -1;
    }
    int i = 0;
    for (pcap_if_t *d = alldevs; d; d = d->next, ++i)
    {
        if (i == FLAGS_i) {
            std::cout << i << ": " << d << std::endl;
            devname = d->name;
            for (const pcap_addr_t *a = d->addresses; a; a = a->next) {
                if (a->netmask && a->netmask->sa_family == AF_INET) {
                    devmask = reinterpret_cast<const sockaddr_in*>(a->netmask)->sin_addr.s_addr;
                    break;
                }
            }
            break;
        }
    }
    pcap_freealldevs(alldevs);
    if (devname.size() == 0) {
        LOG(ERROR) << "invalid adapter index " << FLAGS_i << ", max=" << i-1;
        return -1;
    }

    pcap_t *adhandle;
    if (!(adhandle= pcap_open(devname.c_str(), 65536, PCAP_OPENFLAG_PROMISCUOUS, 1000, NULL, errbuf)))
    {
        LOG(ERROR) << "failed to open the adapter";
        return -1;
    }

    if (FLAGS_f.size() > 0) {
        LOG(INFO) << "set filter \"" << FLAGS_f << "\", netmask=0x" << std::hex 
            << std::setw(8) << std::setfill('0') << ntohl(devmask) << std::dec;
        bpf_program fcode;
        if (pcap_compile(adhandle, &fcode, FLAGS_f.c_str(), 1, devmask) < 0) {
            LOG(ERROR) << "failed to compile the packet filter"; 
            return -1;
        }
        if (pcap_setfilter(adhandle, &fcode) < 0) {
            LOG(ERROR) << "failed to set filter"; 
            return -1;
        }   
    }

    LOG(INFO) << "listening on adapter...";
    int res;
    pcap_pkthdr *header;
    const u_char *pkt_data;
    while((res = pcap_next_ex(adhandle, &header, &pkt_data)) >= 0)
    {
        if (res == 0) {
            VLOG(3) << "timeout elapsed";
            continue;
        }
        print_packet(std::cout, header, pkt_data) << std::endl;
    }

    if (res == -1) {
        LOG(ERROR) << "failed to read packets: " << pcap_geterr(adhandle);
        return -1;
    }
}