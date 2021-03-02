#include "common.h"
#include "uwebsockets/App.h"
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/strand.hpp>
#define CHECK_ERROR_CODE(ec, desc)                                  \
    if (ec) {                                                       \
        LOG(ERROR) << "failed to " << desc << ": " << ec.message(); \
        return;                                                     \
    }

namespace beast = boost::beast;
namespace asio = boost::asio;

class session : public std::enable_shared_from_this<session>
{
public:
    session(asio::io_context &ioc) : resolver_(ioc), ws_(asio::make_strand(ioc)) {}

    void run(const std::string &host, const std::string &port, const std::string &url)
    {
        host_ = host;
        url_ = url;
        resolver_.async_resolve(
            host, port, beast::bind_front_handler(&session::on_resolve, shared_from_this()));
    }

    void on_resolve(beast::error_code ec, asio::ip::tcp::resolver::results_type results)
    {
        CHECK_ERROR_CODE(ec, "resolve");
        beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(30));
        beast::get_lowest_layer(ws_).async_connect(
            results, beast::bind_front_handler(&session::on_connect, shared_from_this()));
    }

    void on_connect(beast::error_code ec, asio::ip::tcp::resolver::results_type::endpoint_type ep)
    {
        CHECK_ERROR_CODE(ec, "connect");
        beast::get_lowest_layer(ws_).expires_never();
        ws_.set_option(beast::websocket::stream_base::timeout::suggested(beast::role_type::client));
        ws_.set_option(
            beast::websocket::stream_base::decorator([](beast::websocket::request_type &req) {
                req.set(beast::http::field::user_agent,
                        std::string(BOOST_BEAST_VERSION_STRING) + " websocket-client-async");
            }));
        host_ += ':' + std::to_string(ep.port());
        ws_.async_handshake(host_, url_,
                            beast::bind_front_handler(&session::on_handshake, shared_from_this()));
    }

    void on_handshake(beast::error_code ec)
    {
        CHECK_ERROR_CODE(ec, "handshake");
        ws_.async_write(asio::buffer("hello websockets!"s),
                        beast::bind_front_handler(&session::on_write, shared_from_this()));
    }

    void on_write(beast::error_code ec, std::size_t bytes_transferred)
    {
        CHECK_ERROR_CODE(ec, "write");
        ws_.async_read(buffer_, beast::bind_front_handler(&session::on_read, shared_from_this()));
    }

    void on_read(beast::error_code ec, std::size_t bytes_transferred)
    {
        CHECK_ERROR_CODE(ec, "read");
        LOG(INFO) << "receive " << bytes_transferred
                  << " bytes: " << beast::make_printable(buffer_.data()) << std::endl;
        buffer_.clear();
        ws_.async_close(beast::websocket::close_code::normal,
                        beast::bind_front_handler(&session::on_close, shared_from_this()));
    }

    void on_close(beast::error_code ec) { CHECK_ERROR_CODE(ec, "close"); }

private:
    asio::ip::tcp::resolver resolver_;
    beast::websocket::stream<beast::tcp_stream> ws_;
    beast::flat_buffer buffer_;
    std::string host_;
    std::string url_;
};

void ws_client()
{
    asio::io_context ioc;
    std::make_shared<session>(ioc)->run("127.0.0.1", "8080", "/");
    ioc.run();
}

void ws_server()
{
    int port = 8080;
    struct UserData
    {
    };
    uWS::App(uWS::SocketContextOptions{})
        .get("/*", [](auto *res, auto *) { res->end("hello http!"); })
        .ws<UserData>("/*", {.message = [](auto *ws, std::string_view message,
                                           uWS::OpCode opCode) { ws->send(message, opCode); }})
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
    std::thread([] { ws_server(); }).detach();
    ws_client();
    NT_CATCH
}
