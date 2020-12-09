#include "prec.h"
#include "core/validator.h"

int main(int argc, char *argv[])
{
    NT_TRY
    google::InitGoogleLogging(argv[0]);
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    FLAGS_logtostderr = 1;
    FLAGS_minloglevel = 0;

    validator::from_str(R"(
        (ipv4.id.0="123" & udp.port = 12) | arp
    )");
    NT_CATCH
}
