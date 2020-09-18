#pragma once
#include "include/cef_app.h"
#include "include/wrapper/cef_message_router.h"

class BasicApp : public CefApp, public CefBrowserProcessHandler, public CefRenderProcessHandler
{
   public:
    BasicApp(const std::wstring &wnd_title, int wnd_width, int wnd_height,
             const std::wstring &app_url)
        : wnd_title_(wnd_title), wnd_width_(wnd_width), wnd_height_(wnd_height), app_url_(app_url)
    {
    }

    // CefApp methods:
    CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override { return this; }
    CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() override { return this; }

    // CefBrowserProcessHandler methods:
    void OnContextInitialized() override;

    // CefRenderProcessHandler methods:
    void OnWebKitInitialized() override;
    void OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                          CefRefPtr<CefV8Context> context) override;
    void OnContextReleased(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                           CefRefPtr<CefV8Context> context) override;
    bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                  CefProcessId source_process,
                                  CefRefPtr<CefProcessMessage> message) override;

   private:
    std::wstring wnd_title_;
    int wnd_width_;
    int wnd_height_;
    std::wstring app_url_;
    CefRefPtr<CefMessageRouterRendererSide> message_router_;
    IMPLEMENT_REFCOUNTING(BasicApp);
    DISALLOW_COPY_AND_ASSIGN(BasicApp);
};