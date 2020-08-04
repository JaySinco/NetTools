#include <winsock2.h>
#include <iphlpapi.h>
#include "dtype.h"

ip4_addr::ip4_addr(u_char c1, u_char c2, u_char c3, u_char c4)
    :b1(c1), b2(c2), b3(c3), b4(c4) {}

ip4_addr::ip4_addr(const std::string &s)
{
    in_addr waddr;
    if (inet_pton(AF_INET, s.c_str(), &waddr) != 1) {
        throw std::runtime_error(nt::sout << "failed to decode ipv4 address: " << s);
    }
    *this = ip4_addr(waddr);
}

ip4_addr::ip4_addr(const in_addr &waddr)
{
    b1 = waddr.S_un.S_un_b.s_b1;
    b2 = waddr.S_un.S_un_b.s_b2;
    b3 = waddr.S_un.S_un_b.s_b3;
    b4 = waddr.S_un.S_un_b.s_b4;
}

ip4_addr::operator in_addr() const
{
    in_addr waddr;
    waddr.S_un.S_un_b.s_b1 = b1;
    waddr.S_un.S_un_b.s_b2 = b2;
    waddr.S_un.S_un_b.s_b3 = b3;
    waddr.S_un.S_un_b.s_b4 = b4;
    return waddr;
}

ip4_addr::operator u_int() const
{
    auto i = reinterpret_cast<const u_int*>(this);
    return *i;
}

bool ip4_addr::operator==(const ip4_addr &other) const
{
    return b1 == other.b1 && b2 == other.b2 &&
        b3 == other.b3 && b4 == other.b4;
}

bool ip4_addr::operator!=(const ip4_addr &other) const
{  
    return !(*this == other);
}

u_int ip4_addr::operator&(const ip4_addr &other) const
{
    auto i = reinterpret_cast<const u_int*>(this);
    auto j = reinterpret_cast<const u_int*>(&other);
    return ntohl(*i) & ntohl(*j);
}

adapter_info::adapter_info(const ip4_addr &subnet_ip, bool exact_match)
{
    u_long buflen = sizeof(IP_ADAPTER_INFO);
    auto pAdapterInfo = reinterpret_cast<IP_ADAPTER_INFO*>(malloc(sizeof(IP_ADAPTER_INFO)));
    if (GetAdaptersInfo(pAdapterInfo, &buflen) == ERROR_BUFFER_OVERFLOW) {
        free(pAdapterInfo);
        pAdapterInfo = reinterpret_cast<IP_ADAPTER_INFO*>(malloc(buflen));
        if (GetAdaptersInfo(pAdapterInfo, &buflen) != NO_ERROR) {
            free(pAdapterInfo);
            throw std::runtime_error(nt::sout << "failed to call GetAdaptersInfo");
        }
    }
    PIP_ADAPTER_INFO pAdapter = NULL;
    pAdapter = pAdapterInfo;
    while (pAdapter) {
        ip4_addr apt_ip(pAdapter->IpAddressList.IpAddress.String);
        ip4_addr apt_mask(pAdapter->IpAddressList.IpMask.String);
        if ((exact_match && apt_ip == subnet_ip) || 
            (!exact_match && (apt_ip & apt_mask) == (subnet_ip & apt_mask)))
        {
            this->name = std::string("\\Device\\NPF_") + pAdapter->AdapterName;
            this->desc = pAdapter->Description;
            this->ip = apt_ip;
            this->mask = apt_mask;
            this->gateway = ip4_addr(pAdapter->GatewayList.IpAddress.String);
            if (pAdapter->AddressLength != sizeof(eth_addr)) {
                LOG(WARNING) << "wrong address length: " << pAdapter->AddressLength;
            }
            else {
                auto c = reinterpret_cast<u_char*>(&this->mac);
                for (unsigned i = 0; i < pAdapter->AddressLength; ++i) {
                    c[i] = pAdapter->Address[i];
                }
            }
            break;
        }
        pAdapter = pAdapter->Next;
    }
    free(pAdapterInfo);
}

std::ostream &operator<<(std::ostream &out, const in_addr &addr)
{
    u_long ip4 = addr.s_addr;
    auto c = reinterpret_cast<const u_char*>(&ip4);
    out << int(c[0]);
    for (int i = 1; i < 4; ++i) {
        out << "." << int(c[i]);
    }
    return out;
}

std::ostream &operator<<(std::ostream &out, const in6_addr &addr)
{
    const u_short *h = addr.u.Word;
    out << std::hex << ntohs(h[0]);
    for (int i = 1; i < 8; ++i) {
        out << ":" << ntohs(h[i]);
    }
    return out << std::dec;
}

std::ostream &operator<<(std::ostream &out, const sockaddr *addr)
{
    if (addr->sa_family == AF_INET) {
        out << reinterpret_cast<const sockaddr_in*>(addr)->sin_addr;
    }
    else if (addr->sa_family == AF_INET6) {
        out << reinterpret_cast<const sockaddr_in6*>(addr)->sin6_addr;
    }
    else if (addr->sa_family == AF_UNSPEC) {
        out << "(unspecified)";
    }
    else {
        LOG(ERROR) << "unknow ip address family: " << addr->sa_family;
    }
    return out;
}

std::ostream &operator<<(std::ostream &out, const ip4_addr &addr)
{
    return out << static_cast<in_addr>(addr);
}

std::ostream &operator<<(std::ostream &out, const eth_addr &addr)
{
    auto c = reinterpret_cast<const u_char*>(&addr);
    out << std::hex << int(c[0]);
    for (int i = 1; i < 6; ++i) {
        out << "-" << int(c[i]);
    }
    return out << std::dec;
}

std::ostream &operator<<(std::ostream &out, const adapter_info &apt)
{
    out << apt.name << std::endl;
    out << "\tDescription: " << apt.desc << std::endl;
    out << "\tMac: " << apt.mac << std::endl;
    out << "\tAddress: " << apt.ip << std::endl;
    out << "\tNetmask: " << apt.mask << std::endl;
    out << "\tGateway: " << apt.gateway << std::endl;
    return out;
}