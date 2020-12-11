#include "net/transport.h"

DEFINE_int32(max, 1500, "high bound for mtu");

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
    LOG(INFO) << "Ping " << ip_desc.str();

    auto &apt = adaptor::fit(ip4::zeros);
    pcap_t *handle = transport::open_adaptor(apt);
    std::shared_ptr<void> handle_guard(nullptr, [&](void *) { pcap_close(handle); });
    int nbytes = transport::calc_mtu(handle, apt, target_ip, FLAGS_max);
    LOG(INFO) << "MTU=" << nbytes;
    NT_CATCH
}
