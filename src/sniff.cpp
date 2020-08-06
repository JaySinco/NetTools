#include "net.h"

DEFINE_string(ip, "", "ipv4 address used to choose adapter");
DEFINE_string(filter, "", "capture filter applied to adapter");

int main(int argc, char* argv[])
{
    NT_TRY
    google::InitGoogleLogging(argv[0]);
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    FLAGS_logtostderr = 1;
    FLAGS_minloglevel = 0;

    if (FLAGS_ip.size() <= 0) {
        LOG(ERROR) << "empty ipv4 address, please set --ip";
        return -1;
    }

    ip4_addr input_ip(FLAGS_ip);
    adapter_info apt_info;
    pcap_t *adhandle = open_target_adaptor(input_ip, false, apt_info);
    std::cout << apt_info << std::endl;

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

    LOG(INFO) << "begin to sniff...";
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
    NT_CATCH
}