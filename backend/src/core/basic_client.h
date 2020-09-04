#pragma once
#include <list>
#include "include/cef_client.h"
#include "include/wrapper/cef_message_router.h"

class BasicClient: public CefClient,
                   public CefDisplayHandler,
                   public CefLifeSpanHandler,
                   public CefRequestHandler,
                   public CefKeyboardHandler,
                   public CefLoadHandler
{
public:
    BasicClient() {}

    // CefClient methods:
    CefRefPtr<CefDisplayHandler> GetDisplayHandler() override { return this; }
    CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override { return this; }
    CefRefPtr<CefRequestHandler> GetRequestHandler() override { return this; }
    CefRefPtr<CefKeyboardHandler> GetKeyboardHandler() override { return this; }
    CefRefPtr<CefLoadHandler> GetLoadHandler() override { return this; }
    bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
        CefProcessId source_process, CefRefPtr<CefProcessMessage> message) override;

    // CefDisplayHandler methods:
    void OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title) override;

    // CefLifeSpanHandler methods:
    void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
    bool DoClose(CefRefPtr<CefBrowser> browser) override;
    void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;
    bool OnBeforePopup(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& target_url,
        const CefString& target_frame_name, CefLifeSpanHandler::WindowOpenDisposition target_disposition, bool user_gesture,
        const CefPopupFeatures& popupFeatures, CefWindowInfo& windowInfo, CefRefPtr<CefClient>& client,
        CefBrowserSettings& settings, CefRefPtr<CefDictionaryValue>& extra_info, bool* no_javascript_access);

    // CefRequestHandler methods:
    bool OnOpenURLFromTab(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& target_url,
        CefRequestHandler::WindowOpenDisposition target_disposition, bool user_gesture) override;
    bool OnBeforeBrowse(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
        CefRefPtr<CefRequest> request, bool user_gesture, bool is_redirect) override;
    void OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser, TerminationStatus status) override;

    // CefKeyboardHandler methods:
    bool OnKeyEvent(CefRefPtr<CefBrowser> browser, const CefKeyEvent& event, CefEventHandle os_event);

    // CefLoadHandler methods:
    void OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, ErrorCode errorCode,
        const CefString& errorText, const CefString& failedUrl) override;

private:
    std::string GetDataURI(const std::string& data, const std::string& mime_type);

    std::list<CefRefPtr<CefBrowser>> browser_list_;
    CefRefPtr<CefMessageRouterBrowserSide> message_router_;
    scoped_ptr<CefMessageRouterBrowserSide::Handler> message_handler_;
    IMPLEMENT_REFCOUNTING(BasicClient);
    DISALLOW_COPY_AND_ASSIGN(BasicClient);
};
