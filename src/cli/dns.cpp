#include "net/transport.h"

DEFINE_string(ip, "8.8.8.8", "dns server ip");

int main(int argc, char *argv[])
{
    NT_TRY
    google::InitGoogleLogging(argv[0]);
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    FLAGS_logtostderr = 1;
    FLAGS_minloglevel = 0;

    if (argc < 2) {
        LOG(ERROR) << "empty domain name, please input domain name";
        return -1;
    }

    dns reply;
    if (transport::query_dns(ip4(FLAGS_ip), argv[1], reply)) {
        LOG(INFO) << reply.to_json().dump(3);
    }
    NT_CATCH
}
