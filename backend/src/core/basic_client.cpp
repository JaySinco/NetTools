#include <windows.h>
#include <thread>
#include "include/base/cef_bind.h"
#include "include/cef_app.h"
#include "include/cef_parser.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_window.h"
#include "include/wrapper/cef_closure_task.h"
#include "include/wrapper/cef_helpers.h"
#include "common.h"
#include "basic_client.h"
#include "msg_handler.h"

bool BasicClient::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
    CefProcessId source_process, CefRefPtr<CefProcessMessage> message)
{
    CEF_REQUIRE_UI_THREAD();
    return message_router_->OnProcessMessageReceived(browser, frame, source_process, message);
}

void BasicClient::OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title)
{
    CEF_REQUIRE_UI_THREAD();
    CefWindowHandle hwnd = browser->GetHost()->GetWindowHandle();
    SetWindowTextW(hwnd, std::wstring(title).c_str());
}

void BasicClient::OnAfterCreated(CefRefPtr<CefBrowser> browser)
{
    CEF_REQUIRE_UI_THREAD();
    if (!message_router_) {
        CefMessageRouterConfig config;
        message_router_ = CefMessageRouterBrowserSide::Create(config);
        message_handler_.reset(new MessageHandler);
        message_router_->AddHandler(message_handler_.get(), false);
    }
    browser_list_.push_back(browser);
}

bool BasicClient::DoClose(CefRefPtr<CefBrowser> browser)
{
    CEF_REQUIRE_UI_THREAD();
    return false;
}

void BasicClient::OnBeforeClose(CefRefPtr<CefBrowser> browser)
{
    CEF_REQUIRE_UI_THREAD();
    message_router_->OnBeforeClose(browser);
    for (auto it = browser_list_.begin(); it != browser_list_.end(); ++it) {
        if ((*it)->IsSame(browser)) {
            browser_list_.erase(it);
            break;
        }
    }
    if (browser_list_.empty()) {
        message_router_->RemoveHandler(message_handler_.get());
        message_handler_.reset();
        message_router_ = nullptr;
        CefQuitMessageLoop();
    }
}

bool BasicClient::OnBeforePopup(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& target_url,
    const CefString& target_frame_name, CefLifeSpanHandler::WindowOpenDisposition target_disposition, bool user_gesture,
    const CefPopupFeatures& popupFeatures, CefWindowInfo& windowInfo, CefRefPtr<CefClient>& client,
    CefBrowserSettings& settings, CefRefPtr<CefDictionaryValue>& extra_info, bool* no_javascript_access)
{
    CEF_REQUIRE_UI_THREAD();
    browser->GetMainFrame()->LoadURL(target_url);
    return true;
}

bool BasicClient::OnOpenURLFromTab(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& target_url,
    CefRequestHandler::WindowOpenDisposition target_disposition, bool user_gesture)
{
    CEF_REQUIRE_UI_THREAD();
    return false;
}

bool BasicClient::OnBeforeBrowse(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
    CefRefPtr<CefRequest> request, bool user_gesture, bool is_redirect)
{
    CEF_REQUIRE_UI_THREAD();
    message_router_->OnBeforeBrowse(browser, frame);
    return false;
}

void BasicClient::OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser, TerminationStatus status)
{
    CEF_REQUIRE_UI_THREAD();
    message_router_->OnRenderProcessTerminated(browser);
}

bool BasicClient::OnKeyEvent(CefRefPtr<CefBrowser> browser, const CefKeyEvent& event, CefEventHandle os_event)
{
    CEF_REQUIRE_UI_THREAD();
    if (event.type == KEYEVENT_RAWKEYDOWN) {
        if (event.windows_key_code == VK_F5) {
            browser->Reload();
            return true;
        }
        if (event.windows_key_code == VK_F6) {
            std::thread([=] {
                SHELLEXECUTEINFOW execInfo = { 0 };
                execInfo.cbSize = sizeof(SHELLEXECUTEINFOW);
                execInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
                execInfo.lpVerb = L"open";
                execInfo.lpFile = L"C:\\Program Files\\Git\\bin\\bash.exe";
                execInfo.lpParameters = L"pack.sh";
                execInfo.lpDirectory = get_srcdir().c_str();
                execInfo.nShow = SW_SHOW;
                if (ShellExecuteExW(&execInfo)) {
                    WaitForSingleObject(execInfo.hProcess, INFINITE);
                    browser->Reload();
                }
            }).detach();
            return true;
        }
        if (event.windows_key_code == VK_F12) {
            if (browser->GetHost()->HasDevTools()) {
                browser->GetHost()->CloseDevTools();
            }
            else {
                CefBrowserSettings browser_settings;
                CefWindowInfo window_info;
                HWND parent = browser->GetHost()->GetWindowHandle();
                window_info.SetAsPopup(parent, "DevTools");
                RECT rect;
                GetWindowRect(parent, &rect);
                window_info.x = rect.right;
                window_info.y = rect.top;
                window_info.height = rect.bottom - rect.top;
                window_info.width = static_cast<int>(window_info.height * 1.8);
                CefPoint pt(0, 0);
                browser->GetHost()->ShowDevTools(window_info, nullptr, browser_settings, pt);
            }
            return true;
        }
        if (event.windows_key_code == VK_LEFT && (event.modifiers & EVENTFLAG_ALT_DOWN)) {
            browser->GoBack();
            return true;
        }
        if (event.windows_key_code == VK_RIGHT && (event.modifiers & EVENTFLAG_ALT_DOWN)) {
            browser->GoForward();
            return true;
        }
    }
    return false;
}

void BasicClient::OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, ErrorCode errorCode,
    const CefString& errorText, const CefString& failedUrl)
{
    CEF_REQUIRE_UI_THREAD();
    if (errorCode == ERR_ABORTED)
        return;
    std::stringstream ss;
    ss << "<html><body bgcolor=\"white\"><h2>Failed to load URL " << std::string(failedUrl)
        << " with error " << std::string(errorText) << " (" << errorCode << ").</h2></body></html>";
    frame->LoadURL(GetDataURI(ss.str(), "text/html"));
}

std::string BasicClient::GetDataURI(const std::string& data, const std::string& mime_type)
{
    return "data:" + mime_type + ";base64," + CefURIEncode(
        CefBase64Encode(data.data(), data.size()), false).ToString();
}