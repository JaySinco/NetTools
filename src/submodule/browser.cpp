#include "browser.h"
#include <wrl.h>
#include <thread>
#define WM_ASYNC_CALL WM_USER + 1

using namespace Microsoft::WRL;

browser::browser(const std::wstring &title, const std::pair<int, int> &size, bool show)
{
    VLOG(1) << "browser<{}> start"_format(ws2s(title));
    std::thread([=] {
        std::lock_guard<std::mutex> lock(lock_running);
        WNDCLASS wc = {0};
        wc.lpszClassName = L"NETTOOLS_CRAWL_BROWSER";
        wc.lpfnWndProc = wnd_proc;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        RegisterClass(&wc);
        h_browser = CreateWindow(wc.lpszClassName, title.c_str(), WS_OVERLAPPEDWINDOW,
                                 CW_USEDEFAULT, CW_USEDEFAULT, size.first, size.second, NULL, NULL,
                                 GetModuleHandle(NULL), this);
        if (!h_browser) {
            throw std::runtime_error("failed to create window");
        }
        ShowWindow(h_browser, show);
        HRESULT hr = CreateCoreWebView2EnvironmentWithOptions(
            nullptr, nullptr, nullptr,
            Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
                this, &browser::environment_created)
                .Get());
        if (FAILED(hr)) {
            throw std::runtime_error("failed to create webview environment, hr={}"_format(hr));
        }
        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        status_ = status::CLOSED;
        VLOG(1) << "browser<{}> closed"_format(ws2s(title));
    }).detach();

    while (status_ != status::RUNNING) {
        std::this_thread::sleep_for(10ms);
    }
}

browser::~browser() { close(); }

void browser::close()
{
    if (!is_closed()) {
        PostMessage(h_browser, WM_DESTROY, NULL, NULL);
        wait_utill_closed();
    }
}

void browser::wait_utill_closed() { std::lock_guard<std::mutex> lock(lock_running); }

bool browser::is_closed() const { return status_ == status::CLOSED; }

void browser::navigate(const std::wstring &url) const
{
    task_t task([=] {
        HRESULT hr = wv_window->Navigate(url.c_str());
        if (FAILED(hr)) {
            throw std::runtime_error("failed to navigate to {}, hr={}"_format(ws2s(url), hr));
        }
        return L"";
    });
    auto future = task.get_future();
    async_call(std::move(task));
    future.get();
}

void browser::async_call(task_t &&task) const
{
    if (is_closed()) {
        throw std::runtime_error("failed to execute async task: browser is closed");
    }
    task_t *p_task = new task_t(std::move(task));
    BOOL ok = PostMessage(h_browser, WM_ASYNC_CALL, reinterpret_cast<WPARAM>(p_task), NULL);
    if (!ok) {
        throw std::runtime_error("failed to post message");
    }
}

HRESULT browser::environment_created(HRESULT result, ICoreWebView2Environment *environment)
{
    return environment->CreateCoreWebView2Controller(
        h_browser, Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                       this, &browser::controller_created)
                       .Get());
}

HRESULT browser::controller_created(HRESULT result, ICoreWebView2Controller *controller)
{
    if (controller != nullptr) {
        wv_controller = controller;
        wv_controller->get_CoreWebView2(&wv_window);
    }
    ICoreWebView2Settings *config;
    wv_window->get_Settings(&config);
    config->put_IsScriptEnabled(TRUE);
    config->put_AreDefaultScriptDialogsEnabled(TRUE);
    config->put_IsWebMessageEnabled(TRUE);
    config->put_AreDevToolsEnabled(TRUE);
    RECT rect;
    GetClientRect(h_browser, &rect);
    wv_controller->put_Bounds(rect);
    wv_window->add_NewWindowRequested(
        Callback<ICoreWebView2NewWindowRequestedEventHandler>(this, &browser::new_window_requested)
            .Get(),
        nullptr);
    status_ = status::RUNNING;
    return S_OK;
}

HRESULT browser::new_window_requested(ICoreWebView2 *sender,
                                      ICoreWebView2NewWindowRequestedEventArgs *args)
{
    wchar_t *url;
    args->get_Uri(&url);
    VLOG(3) << "new window requested, url=" << ws2s(url);
    sender->Navigate(url);
    CoTaskMemFree(url);
    args->put_Handled(TRUE);
    return S_OK;
}

LRESULT CALLBACK browser::scoped_wnd_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
        case WM_SIZE:
            if (wv_controller) {
                RECT rect;
                GetClientRect(hwnd, &rect);
                wv_controller->put_Bounds(rect);
            };
            break;
        case WM_DESTROY:
            VLOG(1) << "clean up webview";
            wv_controller = nullptr;
            wv_window = nullptr;
            PostQuitMessage(0);
            return 0;
        case WM_ASYNC_CALL: {
            auto task = std::unique_ptr<task_t>(reinterpret_cast<task_t *>(wParam));
            (*task)();
            return 0;
        }
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK browser::wnd_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_NCCREATE) {
        CREATESTRUCTW *create = reinterpret_cast<CREATESTRUCTW *>(lParam);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(create->lpCreateParams));
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    auto br = reinterpret_cast<browser *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    return br->scoped_wnd_proc(hwnd, msg, wParam, lParam);
}
