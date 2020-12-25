#include "browser.h"

int main(int argc, char *argv[])
{
    NT_TRY
    INIT_LOG(argc, argv);

    browser browser_;
    browser_.start();
    NT_CATCH
}
