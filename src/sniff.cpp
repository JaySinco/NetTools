#include "net.h"

DEFINE_string(ip, "", "ipv4 address used to choose adapter");
DEFINE_string(filter, "", "capture filter applied to adapter");

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
    catch (const std::runtime_error&) {
        LOG(ERROR) << "invalid ipv4 address: " << FLAGS_ip;
        return -1;
    }

    adapter_info apt_info(input_ip, false);
    if (apt_info.name.size() == 0) {
        LOG(ERROR) << "failed to find adapter according to " << input_ip;
        return -1;
    }
    std::cout << apt_info << std::endl;

    pcap_t *adhandle;
    char errbuf[PCAP_ERRBUF_SIZE];
    if (!(adhandle= pcap_open(apt_info.name.c_str(), 65536, PCAP_OPENFLAG_PROMISCUOUS, 1000, NULL, errbuf)))
    {
        LOG(ERROR) << "failed to open the adapter: " << apt_info.name;
        return -1;
    }

    if (FLAGS_filter.size() > 0) {
        LOG(INFO) << "set filter \"" << FLAGS_filter << "\", netmask=" << apt_info.mask;
        bpf_program fcode;
        if (pcap_compile(adhandle, &fcode, FLAGS_filter.c_str(), 1, static_cast<u_int>(apt_info.mask)) < 0) {
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