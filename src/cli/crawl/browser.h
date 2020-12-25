#include "prec.h"
#include "WebView2.h"
#include <wil/com.h>
#include <atomic>
#include <thread>

class browser
{
public:
    void start();

private:
    void create_window();
    void message_loop();
    HRESULT environment_created(HRESULT result, ICoreWebView2Environment *env);
    HRESULT controller_created(HRESULT result, ICoreWebView2Controller *ctrl);
    LRESULT CALLBACK _wnd_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    HWND h_browser = NULL;
    wil::com_ptr<ICoreWebView2Controller> webview_ctrl;
    wil::com_ptr<ICoreWebView2> webview_win;
};
