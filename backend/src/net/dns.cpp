#include "net.h"

int main(int argc, char *argv[])
{
    NT_TRY
    google::InitGoogleLogging(argv[0]);
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    FLAGS_logtostderr = 1;
    FLAGS_minloglevel = 0;

    if (argc < 2) {
        LOG(ERROR) << "empty target name, please iput ip or host name";
        return -1;
    }
    NT_CATCH
}