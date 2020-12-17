#include "net/transport.h"

DEFINE_string(ip, "8.8.8.8", "dns server ip");

int main(int argc, char *argv[])
{
    NT_TRY
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    if (argc < 2) {
        spdlog::error("empty domain name, please input domain name");
        return -1;
    }

    dns reply;
    if (transport::query_dns(ip4(FLAGS_ip), argv[1], reply)) {
        spdlog::info(reply.to_json().dump(3));
    }
    NT_CATCH
}
