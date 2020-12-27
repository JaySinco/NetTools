#include "submodule/browser.h"

int main(int argc, char *argv[])
{
    NT_TRY
    INIT_LOG(argc, argv);

    browser br(L"Crawl");
    br.navigate(L"https://www.bing.com");
    std::this_thread::sleep_for(3s);
    std::wstring sc = L"document.URL";
    auto ret = br.run_script(sc);
    LOG(INFO) << ws2s(ret);
    br.wait_utill_closed();
    NT_CATCH
}
