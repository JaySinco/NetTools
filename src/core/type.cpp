#include "type.h"
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <sstream>
#include <mutex>

const mac mac::zeros = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
const mac mac::broadcast = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

bool mac::operator==(const mac &rhs) const
{
    return b1 == rhs.b1 && b2 == rhs.b2 && b3 == rhs.b3 && b4 == rhs.b4 && b5 == rhs.b5 &&
           b6 == rhs.b6;
}

bool mac::operator!=(const mac &rhs) const { return !(*this == rhs); }

std::string mac::to_str() const
{
    auto c = reinterpret_cast<const u_char *>(this);
    std::ostringstream ss;
    ss << std::hex << int(c[0]);
    for (int i = 1; i < 6; ++i) {
        ss << "-" << int(c[i]);
    }
    return ss.str();
}

const ip4 ip4::zeros = {0x0, 0x0, 0x0, 0x0};
const ip4 ip4::broadcast = {0xff, 0xff, 0xff, 0xff};

ip4::ip4(u_char c1, u_char c2, u_char c3, u_char c4) : b1(c1), b2(c2), b3(c3), b4(c4) {}

ip4::ip4(const std::string &s)
{
    if (!from_dotted_dec(s, this)) {
        throw std::runtime_error(fmt::format("invalid ip4: {}", s));
    }
}

ip4::ip4(const in_addr &addr)
{
    b1 = addr.S_un.S_un_b.s_b1;
    b2 = addr.S_un.S_un_b.s_b2;
    b3 = addr.S_un.S_un_b.s_b3;
    b4 = addr.S_un.S_un_b.s_b4;
}

ip4::operator in_addr() const
{
    in_addr addr;
    addr.S_un.S_un_b.s_b1 = b1;
    addr.S_un.S_un_b.s_b2 = b2;
    addr.S_un.S_un_b.s_b3 = b3;
    addr.S_un.S_un_b.s_b4 = b4;
    return addr;
}

ip4::operator u_int() const
{
    auto i = reinterpret_cast<const u_int *>(this);
    return *i;
}

bool ip4::operator==(const ip4 &rhs) const
{
    return b1 == rhs.b1 && b2 == rhs.b2 && b3 == rhs.b3 && b4 == rhs.b4;
}

bool ip4::operator!=(const ip4 &rhs) const { return !(*this == rhs); }

u_int ip4::operator&(const ip4 &rhs) const
{
    auto i = reinterpret_cast<const u_int *>(this);
    auto j = reinterpret_cast<const u_int *>(&rhs);
    return ntohl(*i) & ntohl(*j);
}

bool ip4::is_local(const ip4 &rhs, const ip4 &mask) const { return (*this & mask) == (rhs & mask); }

bool ip4::from_dotted_dec(const std::string &s, ip4 *ip)
{
    in_addr addr;
    if (inet_pton(AF_INET, s.c_str(), &addr) != 1) {
        return false;
    }
    if (ip) {
        *ip = ip4(addr);
    }
    return true;
}

bool ip4::from_domain(const std::string &s, ip4 *ip)
{
    addrinfo hints = {0};
    hints.ai_family = AF_INET;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = 0;
    hints.ai_socktype = SOCK_STREAM;
    addrinfo *first_addr;
    auto ret = GetAddrInfoA(s.c_str(), nullptr, &hints, &first_addr);
    if (ret != 0 || first_addr == nullptr) {
        return false;
    }
    if (ip) {
        *ip = reinterpret_cast<sockaddr_in *>(first_addr->ai_addr)->sin_addr;
    }
    return true;
}

std::string ip4::to_str() const
{
    auto c = reinterpret_cast<const u_char *>(this);
    std::ostringstream ss;
    ss << int(c[0]);
    for (int i = 1; i < 4; ++i) {
        ss << "." << int(c[i]);
    }
    return ss.str();
}

json adaptor::to_json() const
{
    json j;
    j["name"] = name;
    j["desc"] = desc;
    j["mac"] = mac_.to_str();
    j["ip"] = ip.to_str();
    j["mask"] = mask.to_str();
    j["gateway"] = gateway.to_str();
    return j;
}

const adaptor &adaptor::fit(const ip4 &hint)
{
    auto it = std::find_if(all().begin(), all().end(), [&](const adaptor &apt) {
        return apt.mask != ip4::zeros && apt.gateway != ip4::zeros &&
               (hint != ip4::zeros ? apt.ip.is_local(hint, apt.mask) : true);
    });
    if (it == all().end()) {
        throw std::runtime_error(fmt::format("no local adapter match {}", hint.to_str()));
    }
    return *it;
}

const std::vector<adaptor> &adaptor::all()
{
    static std::once_flag flag;
    static std::vector<adaptor> adapters;
    std::call_once(flag, [&] {
        u_long buflen = sizeof(IP_ADAPTER_INFO);
        auto plist = reinterpret_cast<IP_ADAPTER_INFO *>(malloc(sizeof(IP_ADAPTER_INFO)));
        std::shared_ptr<void> plist_guard(nullptr, [=](void *) { free(plist); });
        if (GetAdaptersInfo(plist, &buflen) == ERROR_BUFFER_OVERFLOW) {
            plist = reinterpret_cast<IP_ADAPTER_INFO *>(malloc(buflen));
            if (GetAdaptersInfo(plist, &buflen) != NO_ERROR) {
                throw std::runtime_error("failed to get adapters info");
            }
        }
        PIP_ADAPTER_INFO pinfo = plist;
        while (pinfo) {
            adaptor apt;
            ip4 ip(pinfo->IpAddressList.IpAddress.String);
            ip4 mask(pinfo->IpAddressList.IpMask.String);
            ip4 gateway(pinfo->GatewayList.IpAddress.String);
            if (ip != ip4::zeros) {
                apt.name = std::string("\\Device\\NPF_") + pinfo->AdapterName;
                apt.desc = pinfo->Description;
                apt.ip = ip;
                apt.mask = mask;
                apt.gateway = gateway;
                if (pinfo->AddressLength != sizeof(mac)) {
                    LOG(WARNING) << "wrong mac length: " << pinfo->AddressLength;
                } else {
                    auto c = reinterpret_cast<u_char *>(&apt.mac_);
                    for (unsigned i = 0; i < pinfo->AddressLength; ++i) {
                        c[i] = pinfo->Address[i];
                    }
                }
                adapters.push_back(apt);
            }
            pinfo = pinfo->Next;
        }
        if (adapters.size() <= 0) {
            throw std::runtime_error("failed to find any suitable adapter");
        }
    });
    return adapters;
}

wsa_guard wsa_guard::g;

wsa_guard::wsa_guard()
{
    WSADATA ws;
    WSAStartup(MAKEWORD(2, 2), &ws);
}

wsa_guard::~wsa_guard() { WSACleanup(); }
