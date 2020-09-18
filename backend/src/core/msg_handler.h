#pragma once
#include "json.hpp"
#include "include/wrapper/cef_message_router.h"

using json = nlohmann::json;

class MessageHandler : public CefMessageRouterBrowserSide::Handler
{
   public:
    MessageHandler() {}

    bool OnQuery(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int64 query_id,
                 const CefString &request, bool persistent, CefRefPtr<Callback> callback) override;

   private:
    // can be called on any thread in the browser process
    void SendIpcMessageToJs(CefRefPtr<CefBrowser> browser, const std::wstring &header,
                            const json &body = json{});

    DISALLOW_COPY_AND_ASSIGN(MessageHandler);
};