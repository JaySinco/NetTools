#include "browser.h"

int main(int argc, char *argv[])
{
    NT_TRY
    INIT_LOG(argc, argv);

    browser br(L"Crawl");
    br.navigate(L"https://www.baidu.com");
    std::this_thread::sleep_for(5s);
    br.navigate(L"https://www.bing.com");
    br.wait_utill_closed();
    NT_CATCH
}
