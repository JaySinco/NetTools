#include "net.h"

int main(int argc, char* argv[])
{
    NT_TRY
    google::InitGoogleLogging(argv[0]);
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    FLAGS_logtostderr = 1;
    FLAGS_minloglevel = 0;

    pcap_if_t *alldevs;
    char errbuf[PCAP_ERRBUF_SIZE];
    if (pcap_findalldevs(&alldevs, errbuf) == -1)
    {
        LOG(ERROR) << "failed to find all device: " << errbuf;
        return -1;
    }
    int i = 0;
    for (pcap_if_t *d = alldevs; d; d = d->next, ++i)
    {
        std::cout << i << ": " << d << std::endl;
    }
    pcap_freealldevs(alldevs);
    NT_CATCH
}