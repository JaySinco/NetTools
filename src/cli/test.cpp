#include "prec.h"
#include "net/transport.h"

int main(int argc, char *argv[])
{
    NT_TRY
    INIT_LOG(argc, argv);

    LOG(INFO) << "test";
    NT_CATCH
}
