#include "core/transport.h"
#include "core/packet.h"

DEFINE_string(ip, "", "ipv4 address used to choose adapter, select first if empty");
DEFINE_string(filter, "", "capture filter applied to adapter");

int main(int argc, char *argv[])
{
    NT_TRY
    google::InitGoogleLogging(argv[0]);
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    FLAGS_logtostderr = 1;
    FLAGS_minloglevel = 0;

    auto &apt = adaptor::fit(FLAGS_ip.size() > 0 ? ip4(FLAGS_ip) : ip4::zeros);
    pcap_t *handle = transport::open_adaptor(apt);

    LOG(INFO) << apt.to_json().dump(3);

    if (FLAGS_filter.size() > 0) {
        LOG(INFO) << "set filter \"" << FLAGS_filter << "\", mask=" << apt.mask.to_str();
        bpf_program fcode;
        if (pcap_compile(handle, &fcode, FLAGS_filter.c_str(), 1, static_cast<u_int>(apt.mask)) <
            0) {
            LOG(ERROR) << "failed to compile the packet filter, please refer to "
                          "https://nmap.org/npcap/guide/wpcap/pcap-filter.html";
            return -1;
        }
        if (pcap_setfilter(handle, &fcode) < 0) {
            LOG(ERROR) << "failed to set filter";
            return -1;
        }
    }

    LOG(INFO) << "begin to sniff...";
    int res;
    pcap_pkthdr *info;
    const u_char *start;
    while ((res = pcap_next_ex(handle, &info, &start)) >= 0) {
        if (res == 0) {
            continue;  // timeout elapsed
        }
        LOG(INFO) << packet(start, start + info->len).to_json().dump(3);
    }
    if (res == -1) {
        throw std::runtime_error(fmt::format("failed to read packets: {}", pcap_geterr(handle)));
    }
    NT_CATCH
}
