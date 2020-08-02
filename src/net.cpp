#include "net.h"

std::ostream &operator<<(std::ostream &out, const sockaddr *addr)
{
    if (addr->sa_family == AF_INET) {
        u_long ipv4 = reinterpret_cast<const sockaddr_in*>(addr)->sin_addr.s_addr;
        const u_char *c = reinterpret_cast<u_char*>(&ipv4);
        out << int(c[0]);
        for (int i = 1; i < 4; ++i) {
            out << "." << int(c[i]);
        }
    }
    else if (addr->sa_family == AF_INET6) {
        const u_short *h = reinterpret_cast<const sockaddr_in6*>(addr)->sin6_addr.u.Word;
        out << std::hex << ntohs(h[0]);
        for (int i = 1; i < 8; ++i) {
            out << ":" << ntohs(h[i]);
        }
        out << std::dec;
    }
    else if (addr->sa_family == AF_UNSPEC) {
        out << "(unspecified)";
    }
    else {
        out << "(unknow family: " << addr->sa_family << ")";
    }
    return out;
}

std::ostream &operator<<(std::ostream &out, const pcap_if_t *dev)
{
    out << dev->name << std::endl;
    out << "\tDescription: " << dev->description << std::endl;
    out << "\tLoopback: " << ((dev->flags & PCAP_IF_LOOPBACK) ? "yes" : "no") << std::endl;
    for (pcap_addr_t *a = dev->addresses; a; a = a->next) {
        out << "\t---------------------------------------" << std::endl;
        if (a->addr) out << "\tAddress: " << a->addr<< std::endl;
        if (a->netmask) out << "\tNetmask: " << a->netmask << std::endl;
        if (a->broadaddr) out << "\tBroadcast: " << a->broadaddr << std::endl;
        if (a->dstaddr) out << "\tDestination: " << a->dstaddr << std::endl;
    }
    return out;
}

std::ostream &print_packet(std::ostream &out, const pcap_pkthdr *header, const u_char *data)
{
    time_t local_tv_sec = header->ts.tv_sec;
    tm ltime;
    localtime_s(&ltime, &local_tv_sec);
    char timestr[16];
    strftime(timestr, sizeof(timestr), "%H:%M:%S", &ltime);
    out << timestr << "." << std::setw(6) << std::left << header->ts.tv_usec;
    out << "  len: " << header->len;
    return out;
}