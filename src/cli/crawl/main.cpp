#include "browser.h"

int main(int argc, char *argv[])
{
    NT_TRY
    INIT_LOG(argc, argv);

    browser br;
    br.start();
    std::this_thread::sleep_for(5s);
    br.close();
    std::this_thread::sleep_for(1s);
    NT_CATCH
}
