#include "browser.h"
#include <wrl.h>

using namespace Microsoft::WRL;

void browser::start() { message_loop(); }

void browser::create_window()
{
    WNDCLASS wc = {0};
    wc.lpszClassName = L"NETTOOLS_CRAWL_BROWSER";
    wc.lpfnWndProc = wnd_proc;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    if (!RegisterClass(&wc)) {
        throw std::runtime_error("failed to register class");
    }
    h_browser = CreateWindow(wc.lpszClassName, L"Crawl", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
                             CW_USEDEFAULT, 1200, 900, NULL, NULL, GetModuleHandle(NULL), this);
    if (!h_browser) {
        throw std::runtime_error("failed to create window");
    }
    ShowWindow(h_browser, true);
}

void browser::message_loop()
{
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
    LOG(INFO) << "quit message loop";
}

HRESULT browser::environment_created(HRESULT result, ICoreWebView2Environment *env)
{
    return env->CreateCoreWebView2Controller(
        h_browser, Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                       this, &browser::controller_created)
                       .Get());
}

HRESULT browser::controller_created(HRESULT result, ICoreWebView2Controller *ctrl)
{
    if (ctrl != nullptr) {
        webview_ctrl = ctrl;
        webview_ctrl->get_CoreWebView2(&webview_win);
    }

    // Add a few settings for the webview
    // The demo step is redundant since the values are the default settings
    ICoreWebView2Settings *Settings;
    webview_win->get_Settings(&Settings);
    Settings->put_IsScriptEnabled(TRUE);
    Settings->put_AreDefaultScriptDialogsEnabled(TRUE);
    Settings->put_IsWebMessageEnabled(TRUE);

    // Resize WebView to fit the bounds of the parent window
    RECT bounds;
    GetClientRect(h_browser, &bounds);
    webview_ctrl->put_Bounds(bounds);

    // Schedule an async task to navigate to Bing
    webview_win->Navigate(L"https://www.google.com/");

    // Step 4 - Navigation events

    // Step 5 - Scripting

    // Step 6 - Communication between host and web content

    return S_OK;
}

LRESULT CALLBACK browser::_wnd_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
        case WM_SIZE:
            if (webview_ctrl) {
                RECT rect;
                GetClientRect(hwnd, &rect);
                webview_ctrl->put_Bounds(rect);
            };
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK browser::wnd_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_NCCREATE) {
        CREATESTRUCTW *create = (CREATESTRUCTW *)lParam;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)create->lpCreateParams);
        SetWindowText(hwnd, create->lpszName);
        return TRUE;
    }
    return ((browser *)GetWindowLongPtr(hwnd, GWLP_USERDATA))->_wnd_proc(hwnd, msg, wParam, lParam);
}
