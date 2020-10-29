#include "core/transport.h"
#include <iostream>

DEFINE_string(ip, "", "ipv4 address used to choose adapter, select first if empty");
DEFINE_string(filter, "", "capture filter applied to adapter");

int main(int argc, char *argv[])
{
    NT_TRY
    google::InitGoogleLogging(argv[0]);
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    FLAGS_logtostderr = 1;
    FLAGS_minloglevel = 0;

    ip4 input_ip = argc >= 2 ? ip4(argv[1]) : ip4::zeros;
    auto &apt = adaptor::fit(input_ip);
    pcap_t *handle = transport::open_adaptor(apt);
    packet req = packet::arp(input_ip), reply;
    if (transport::send_and_recv(handle, req, reply, 5000)) {
        json layers = reply.to_json()["layers"];
        json mac = layers.back()["source-mac"];
        std::cout << input_ip.to_str() << " is at " << mac.get<std::string>() << "." << std::endl;
    } else {
        std::cout << input_ip.to_str() << " is offline." << std::endl;
    }
    NT_CATCH
}
