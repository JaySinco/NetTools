#include "prec.h"
#include <WebView2.h>
#include <wil/com.h>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <future>

class browser
{
public:
    browser(const std::wstring &title, const std::pair<int, int> &size = {1200, 900},
            bool show = true);

    void close() const;
    void wait_utill_closed();
    bool is_closed() const;
    void navigate(const std::wstring &url) const;

private:
    using task_t = std::packaged_task<std::wstring()>;
    enum class status
    {
        INITIAL,
        RUNNING,
        CLOSED,
    };

    void async_call(task_t &&task) const;
    HRESULT environment_created(HRESULT result, ICoreWebView2Environment *environment);
    HRESULT controller_created(HRESULT result, ICoreWebView2Controller *controller);
    LRESULT scoped_wnd_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static LRESULT wnd_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    HWND h_browser = NULL;
    DWORD thread_id = 0;
    std::mutex lock_running;
    std::atomic<status> status_ = status::INITIAL;
    wil::com_ptr<ICoreWebView2Controller> wv_controller;
    wil::com_ptr<ICoreWebView2> wv_window;
};
