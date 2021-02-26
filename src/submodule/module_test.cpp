#include "submodule/browser.h"
#pragma warning(disable : 4138)
#include "uwebsockets/App.h"

void test_browser()
{
    browser br(L"test");
    br.navigate(L"https://www.baidu.com");
    br.wait_utill_closed();
}

void test_websocket(int port)
{
    uWS::App(uWS::SocketContextOptions{})
        .get("/*", [](auto *res, auto * /*req*/) { res->end("Hello world!"); })
        .listen("127.0.0.1", port,
                [=](auto *listen_socket) {
                    if (listen_socket) {
                        LOG(INFO) << "listening on port " << port;
                    }
                })
        .run();

    LOG(ERROR) << "failed to listen on port " << port;
}

int main(int argc, char *argv[])
{
    NT_TRY
    INIT_LOG(argc, argv);
    // test_browser();
    test_websocket(8000);
    NT_CATCH
}
