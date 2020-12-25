#include "prec.h"
#include "WebView2.h"
#include <wil/com.h>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <thread>

class browser
{
public:
    void start();
    void close();

private:
    void create_window();
    void message_loop();
    HRESULT environment_created(HRESULT result, ICoreWebView2Environment *environment);
    HRESULT controller_created(HRESULT result, ICoreWebView2Controller *controller);
    LRESULT CALLBACK scoped_wnd_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    HWND h_browser = NULL;
    DWORD thread_id = 0;
    wil::com_ptr<ICoreWebView2Controller> wv_controller;
    wil::com_ptr<ICoreWebView2> wv_window;
};
