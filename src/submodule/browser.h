#pragma once
#include "common.h"
#include <WebView2.h>
#include <wil/com.h>
#include <atomic>
#include <mutex>
#include <future>

class browser
{
public:
    browser(const std::wstring &title, const std::pair<int, int> &size = {1200, 900},
            bool show = true);

    ~browser();
    void wait_utill_closed();
    void close();
    bool is_closed() const;
    void navigate(const std::wstring &url) const;
    std::wstring run_script(const std::wstring &source);

private:
    using task_t = std::packaged_task<std::wstring()>;
    enum class status
    {
        INITIAL,
        RUNNING,
        CLOSED,
    };

    void post_task(task_t &&task) const;
    HRESULT environment_created(HRESULT result, ICoreWebView2Environment *environment);
    HRESULT controller_created(HRESULT result, ICoreWebView2Controller *controller);
    HRESULT new_window_requested(ICoreWebView2 *sender,
                                 ICoreWebView2NewWindowRequestedEventArgs *args);
    LRESULT scoped_wnd_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static LRESULT wnd_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    HWND h_browser = NULL;
    std::mutex lock_running;
    std::atomic<status> status_ = status::INITIAL;
    wil::com_ptr<ICoreWebView2Controller> wv_controller = nullptr;
    wil::com_ptr<ICoreWebView2> wv_window = nullptr;
};
