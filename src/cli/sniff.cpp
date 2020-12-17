#include "net/transport.h"
#include "net/validator.h"

DEFINE_string(ip, "", "ipv4 address used to choose adapter, select first if empty");
DEFINE_string(filter, "", "capture filter applied at runtime");
DEFINE_string(driver_filter, "", "capture filter applied to adapter driver");

int main(int argc, char *argv[])
{
    NT_TRY
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    auto &apt = adaptor::fit(FLAGS_ip.size() > 0 ? ip4(FLAGS_ip) : ip4::zeros);
    pcap_t *handle = transport::open_adaptor(apt);
    std::shared_ptr<void> handle_guard(nullptr, [&](void *) { pcap_close(handle); });
    spdlog::info(apt.to_json().dump(3));

    if (FLAGS_driver_filter.size() > 0) {
        spdlog::info(R"(set driver filter "{}", mask={})", FLAGS_driver_filter, apt.mask.to_str());
        transport::setfilter(handle, FLAGS_driver_filter, apt.mask);
    }

    p_validator validator_;
    if (FLAGS_filter.size() > 0) {
        spdlog::info(R"(set filter "{}")", FLAGS_filter);
        validator_ = validator::from_str(FLAGS_filter);
    }

    spdlog::info("begin to sniff...");
    transport::recv(handle, [&](const packet &p) {
        if (!validator_ || validator_->test(p)) {
            spdlog::info(p.to_json()["layers"].dump(3));
        }
        return false;
    });
    NT_CATCH
}
