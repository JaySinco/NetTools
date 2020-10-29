#include "core/transport.h"

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
        transport::set_filter(handle, FLAGS_filter, apt.mask);
    }

    LOG(INFO) << "begin to sniff...";
    transport::recv(handle, [](const packet &p) {
        LOG(INFO) << p.to_json()["layers"].dump(3);
        return false;
    });
    NT_CATCH
}
