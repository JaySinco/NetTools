#include "include/cef_browser.h"
#include "include/cef_command_line.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_window.h"
#include "include/wrapper/cef_helpers.h"
#include "basic_app.h"
#include "basic_client.h"

void BasicApp::OnContextInitialized()
{
    CEF_REQUIRE_UI_THREAD();
    auto command_line = CefCommandLine::GetGlobalCommandLine();
    auto url = command_line->GetSwitchValue("url");
    if (url.empty()) {
        url = app_url_;
    }
    CefRefPtr<BasicClient> handler(new BasicClient);
    CefBrowserSettings browser_settings;
    CefWindowInfo window_info;
    window_info.SetAsPopup(NULL, wnd_title_);
    window_info.width = wnd_width_;
    window_info.height = wnd_height_;
    CefBrowserHost::CreateBrowser(window_info, handler, url, browser_settings, nullptr, nullptr);
}

void BasicApp::OnWebKitInitialized()
{
    CefMessageRouterConfig config;
    message_router_ = CefMessageRouterRendererSide::Create(config);
}

void BasicApp::OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                CefRefPtr<CefV8Context> context)
{
    message_router_->OnContextCreated(browser, frame, context);
}

void BasicApp::OnContextReleased(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                 CefRefPtr<CefV8Context> context)
{
    message_router_->OnContextReleased(browser, frame, context);
}

bool BasicApp::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                        CefProcessId source_process,
                                        CefRefPtr<CefProcessMessage> message)
{
    if (!message_router_->OnProcessMessageReceived(browser, frame, source_process, message)) {
        if (message->GetName() == L"SendIpcMessageToJs") {
            std::ostringstream ss;
            ss << "onReceiveIpcMessage(`" << message->GetArgumentList()->GetString(0) << "`);";
            browser->GetMainFrame()->ExecuteJavaScript(ss.str(), frame->GetURL(), 0);
        } else {
            return false;
        }
    }
    return true;
}
