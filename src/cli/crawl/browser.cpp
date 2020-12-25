#include "browser.h"
#include <wrl.h>

using namespace Microsoft::WRL;

void browser::start()
{
    VLOG(1) << "start browser";
    std::thread(&browser::message_loop, this).detach();
}

void browser::close()
{
    VLOG(1) << "close browser";
    PostThreadMessage(thread_id, WM_QUIT, 0, NULL);
}

void browser::create_window()
{
    WNDCLASS wc = {0};
    wc.lpszClassName = L"NETTOOLS_CRAWL_BROWSER";
    wc.lpfnWndProc = wnd_proc;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClass(&wc);
    h_browser = CreateWindow(wc.lpszClassName, L"Crawl", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
                             CW_USEDEFAULT, 1200, 900, NULL, NULL, GetModuleHandle(NULL), this);
    if (!h_browser) {
        throw std::runtime_error("failed to create window");
    }
    ShowWindow(h_browser, true);
}

void browser::message_loop()
{
    VLOG(1) << "message-loop start";
    thread_id = GetCurrentThreadId();
    create_window();
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
    VLOG(1) << "message-loop quit";
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

    // Add a few settings for the webview
    // The demo step is redundant since the values are the default settings
    ICoreWebView2Settings *Settings;
    wv_window->get_Settings(&Settings);
    Settings->put_IsScriptEnabled(TRUE);
    Settings->put_AreDefaultScriptDialogsEnabled(TRUE);
    Settings->put_IsWebMessageEnabled(TRUE);

    // Resize WebView to fit the bounds of the parent window
    RECT bounds;
    GetClientRect(h_browser, &bounds);
    wv_controller->put_Bounds(bounds);

    // Schedule an async task to navigate to Bing
    wv_window->Navigate(L"https://www.google.com/");

    // Step 4 - Navigation events

    // Step 5 - Scripting

    // Step 6 - Communication between host and web content

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
            PostQuitMessage(0);
            return 0;
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
