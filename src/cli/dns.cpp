#include "core/transport.h"
#include "core/dns.h"

DEFINE_string(ip, "8.8.8.8", "dns server ip");

int main(int argc, char *argv[])
{
    NT_TRY
    google::InitGoogleLogging(argv[0]);
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    FLAGS_logtostderr = 1;
    FLAGS_minloglevel = 0;

    if (argc < 2) {
        LOG(ERROR) << "empty domain name, please input domain name";
        return -1;
    }

    SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    std::shared_ptr<void *> socket_guard(nullptr, [=](void *) {
        if (closesocket(s) == SOCKET_ERROR) {
            LOG(ERROR) << "failed to close socket!";
        };
    });
    if (s == INVALID_SOCKET) {
        throw std::runtime_error("failed to create udp socket");
    }
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(53);
    if (inet_pton(AF_INET, FLAGS_ip.c_str(), &addr.sin_addr) != 1) {
        throw std::runtime_error(fmt::format("failed to decode dns server: {}", FLAGS_ip));
    }
    dns query(argv[1]);
    std::vector<u_char> packet;
    query.to_bytes(packet);
    if (sendto(s, reinterpret_cast<const char *>(packet.data()), static_cast<int>(packet.size()), 0,
               reinterpret_cast<sockaddr *>(&addr), sizeof(sockaddr_in)) == SOCKET_ERROR) {
        throw std::runtime_error(fmt::format("failed to send dns data: {}", WSAGetLastError()));
    }
    char buf[1024] = {0};
    sockaddr_in from;
    int from_len = sizeof(sockaddr_in);
    int recv_len = recvfrom(s, buf, sizeof(buf), 0, reinterpret_cast<sockaddr *>(&from), &from_len);
    if (recv_len == SOCKET_ERROR) {
        throw std::runtime_error(fmt::format("failed to receive dns data: {}", WSAGetLastError()));
    }
    const u_char *start = reinterpret_cast<u_char *>(buf);
    const u_char *end = start + recv_len;
    dns reply(start, end);
    LOG(INFO) << reply.to_json().dump(3);
    NT_CATCH
}
