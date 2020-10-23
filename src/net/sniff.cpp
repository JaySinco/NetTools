#include "net.h"

DEFINE_string(ip, "", "ipv4 address used to choose adapter, select-auto if empty");
DEFINE_string(filter, "", "capture filter applied to adapter");

int main(int argc, char *argv[])
{
    NT_TRY
    google::InitGoogleLogging(argv[0]);
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    FLAGS_logtostderr = 1;
    FLAGS_minloglevel = 0;

    ip4_addr input_ip = PLACEHOLDER_IPv4_ADDR;
    if (FLAGS_ip.size() > 0) {
        input_ip = ip4_addr(FLAGS_ip);
    }
    adapter_info apt_info;
    pcap_t *adhandle = open_target_adaptor(input_ip, false, apt_info);
    std::cout << apt_info << std::endl;

    if (FLAGS_filter.size() > 0) {
        LOG(INFO) << "set filter \"" << FLAGS_filter << "\", netmask=" << apt_info.mask;
        bpf_program fcode;
        if (pcap_compile(adhandle, &fcode, FLAGS_filter.c_str(), 1,
                         static_cast<u_int>(apt_info.mask)) < 0) {
            LOG(ERROR) << "failed to compile the packet filter, please refer to "
                          "https://nmap.org/npcap/guide/wpcap/pcap-filter.html";
            return -1;
        }
        if (pcap_setfilter(adhandle, &fcode) < 0) {
            LOG(ERROR) << "failed to set filter";
            return -1;
        }
    }

    LOG(INFO) << "begin to sniff...";
    auto start_tm = std::chrono::system_clock::now();
    packet_loop(adhandle, start_tm, -1, [&](pcap_pkthdr *pkthdr, const ethernet_header *eh) {
        print_packet(std::cout, pkthdr, eh) << std::endl;
        return NTLS_CONTINUE;
    });
    NT_CATCH
}
