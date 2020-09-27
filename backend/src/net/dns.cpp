#include <winsock2.h>
#include "net.h"

void dns_client(const std::string &domain, const std::string &dns_server)
{
    SOCKET s = socket(AF_INET, SOCK_DGRAM, IPv4_TYPE_UDP);
    std::shared_ptr<void *> socket_guarder(nullptr, [=](void *) {
        if (closesocket(s) == SOCKET_ERROR) {
            LOG(ERROR) << "failed to close socket!";
        };
        VLOG(1) << "socket closed!";
    });
    if (s == INVALID_SOCKET) {
        throw std::runtime_error("failed to create udp socket");
    }
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(53);
    if (inet_pton(AF_INET, dns_server.c_str(), &addr.sin_addr) != 1) {
        throw std::runtime_error(fmt::format("failed to decode dns domain: {}", dns_server));
    }
    u_short id = 0;
    Bytes packet = make_dns_query(domain, id);
    VLOG(1) << "dns id=" << id;
    if (sendto(s, reinterpret_cast<const char *>(packet.data()), static_cast<int>(packet.size()), 0,
               reinterpret_cast<sockaddr *>(&addr), sizeof(sockaddr_in)) == SOCKET_ERROR) {
        throw std::runtime_error(fmt::format("failed to send dns data: {}", WSAGetLastError()));
    }
    char buf[1000] = {0};
    sockaddr_in from;
    int from_len = sizeof(sockaddr_in);
    int recv_len = recvfrom(s, buf, sizeof(buf), 0, reinterpret_cast<sockaddr *>(&from), &from_len);
    if (recv_len == SOCKET_ERROR) {
        throw std::runtime_error(fmt::format("failed to receive dns data: {}", WSAGetLastError()));
    }
    VLOG(1) << "got " << recv_len << " bytes!";
    dns_reply reply = parse_dns_reply(Bytes(buf, buf + recv_len));
    std::cout << reply << std::endl;
}

int main(int argc, char *argv[])
{
    NT_TRY
    google::InitGoogleLogging(argv[0]);
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    FLAGS_logtostderr = 1;
    FLAGS_minloglevel = 0;

    dns_client("www.baidu.com", "223.5.5.5");

    NT_CATCH
}