#include "net/transport.h"
#include "net/validator.h"

DEFINE_string(ip, "", "ipv4 address used to choose adapter, select first if empty");
DEFINE_string(filter, "", "capture filter applied at runtime");
DEFINE_string(driver_filter, "", "capture filter applied to adapter driver");

int main(int argc, char *argv[])
{
    NT_TRY
    google::InitGoogleLogging(argv[0]);
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    FLAGS_logtostderr = 1;
    FLAGS_minloglevel = 0;

    auto &apt = adaptor::fit(FLAGS_ip.size() > 0 ? ip4(FLAGS_ip) : ip4::zeros);
    pcap_t *handle = transport::open_adaptor(apt);
    std::shared_ptr<void> handle_guard(nullptr, [&](void *) { pcap_close(handle); });
    LOG(INFO) << apt.to_json().dump(3);

    if (FLAGS_driver_filter.size() > 0) {
        LOG(INFO) << "set driver filter \"" << FLAGS_driver_filter
                  << "\", mask=" << apt.mask.to_str();
        transport::setfilter(handle, FLAGS_driver_filter, apt.mask);
    }

    p_validator validator_;
    if (FLAGS_filter.size() > 0) {
        LOG(INFO) << "set filter \"" << FLAGS_driver_filter << "\"";
        validator_ = validator::from_str(FLAGS_filter);
    }

    LOG(INFO) << "begin to sniff...";
    transport::recv(handle, [&](const packet &p) {
        if (!validator_ || validator_->test(p)) {
            LOG(INFO) << p.to_json()["layers"].dump(3);
        }
        return false;
    });
    NT_CATCH
}
