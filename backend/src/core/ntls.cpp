#include <windows.h>
#include <sstream>
#include "common.h"
#include "basic_app.h"

std::wstring calc_app_url()
{
    std::wostringstream ss;
    ss << L"file://" << get_resdir() << L"\\index.html";
    return ss.str();
}

int wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    CefEnableHighDPISupport();
    CefMainArgs main_args(hInstance);
    CefRefPtr<BasicApp> app(new BasicApp(L"ntls", 800, 600, calc_app_url()));
    int exit_code = CefExecuteProcess(main_args, app, nullptr);
    if (exit_code >= 0) {
        return exit_code;
    }
    CefSettings settings;
    settings.no_sandbox = true;
    settings.remote_debugging_port = 8081;
    CefInitialize(main_args, settings, app, nullptr);
    CefRunMessageLoop();
    CefShutdown();
    return 0;
}