#include "core/transport.h"

int main(int argc, char *argv[])
{
    NT_TRY
    google::InitGoogleLogging(argv[0]);
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    FLAGS_logtostderr = 1;
    FLAGS_minloglevel = 0;

    json j;
    for (const auto &apt : adaptor::all()) {
        j.push_back(apt.to_json());
    }
    LOG(INFO) << j.dump(3);
    NT_CATCH
}
