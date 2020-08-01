#include "net.h"

std::ostream &operator<<(std::ostream &o, const sockaddr *s)
{
    if (s->sa_family == AF_INET) {
        return o << inet_ntoa(((sockaddr_in*)s)->sin_addr);
    }
    return o;
}

std::ostream &operator<<(std::ostream &o, const pcap_if_t *p)
{
    o << p->name << std::endl;
    o << "\tDescription: " << p->description << std::endl;
    o << "\tLoopback: " << ((p->flags & PCAP_IF_LOOPBACK) ? "yes" : "no") << std::endl;
    for (pcap_addr_t *a = p->addresses; a; a = a->next) {
        if (a->addr->sa_family == AF_INET) {
            if (a->addr) o << "\tAddress: " << a->addr<< std::endl;
            if (a->netmask) o << "\tNetmask: " << a->netmask << std::endl;
            if (a->broadaddr) o << "\tBroadcast: " << a->broadaddr << std::endl;
            if (a->dstaddr) o << "\tDestination: " << a->dstaddr << std::endl;
        }
    }
    return o;
}