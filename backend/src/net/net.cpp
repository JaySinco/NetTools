#include <winsock2.h>
#include "net.h"
#include <thread>
#include <atomic>
#include <map>
#include <random>
#include <iomanip>
#include <iphlpapi.h>

static wsa_guard g_wsa_guarder;

u_short rand_ushort()
{
    static std::random_device rd;
    static std::default_random_engine engine(rd());
    static std::uniform_int_distribution<u_short> dist;
    return dist(engine);
}

wsa_guard::wsa_guard() {
    WSADATA ws;
    WSAStartup(MAKEWORD(2, 2), &ws);
}

wsa_guard::~wsa_guard() {
    WSACleanup();
}

pcap_t *open_target_adaptor(const ip4_addr &ip, bool exact_match, adapter_info &apt_info)
{
    apt_info = adapter_info::select_ip(ip, exact_match);
    if (apt_info.name.size() == 0) {
        throw std::runtime_error(fmt::format("no adapter found in same local network with {}", to_string(ip)));
    }
    pcap_t *adhandle;
    char errbuf[PCAP_ERRBUF_SIZE];
    if (!(adhandle= pcap_open(apt_info.name.c_str(), 65536, PCAP_OPENFLAG_PROMISCUOUS, 1000, NULL, errbuf)))
    {
        throw std::runtime_error(fmt::format("failed to open the adapter: {}", apt_info.name));
    }
    return adhandle;
}

adapter_info adapter_info::select_ip(const ip4_addr &subnet_ip, bool exact_match)
{
    adapter_info apt_info;
    u_long buflen = sizeof(IP_ADAPTER_INFO);
    auto pAdapterInfo = reinterpret_cast<IP_ADAPTER_INFO*>(malloc(sizeof(IP_ADAPTER_INFO)));
    std::shared_ptr<void> pAdapterInfo_guard(nullptr, [=](void *){ free(pAdapterInfo); });
    if (GetAdaptersInfo(pAdapterInfo, &buflen) == ERROR_BUFFER_OVERFLOW) {
        pAdapterInfo = reinterpret_cast<IP_ADAPTER_INFO*>(malloc(buflen));
        if (GetAdaptersInfo(pAdapterInfo, &buflen) != NO_ERROR) {
            throw std::runtime_error("failed to call GetAdaptersInfo");
        }
    }
    bool select_auto = (subnet_ip == PLACEHOLDER_IPv4_ADDR);
    PIP_ADAPTER_INFO pAdapter = NULL;
    pAdapter = pAdapterInfo;
    bool found = false;
    while (pAdapter) {
        ip4_addr apt_ip(pAdapter->IpAddressList.IpAddress.String);
        ip4_addr apt_mask(pAdapter->IpAddressList.IpMask.String);
        ip4_addr apt_gateway(pAdapter->GatewayList.IpAddress.String);
        bool auto_cond = (apt_gateway != PLACEHOLDER_IPv4_ADDR);
        bool exact_match_cond = (exact_match && apt_ip == subnet_ip);
        bool subnet_cond = (!exact_match && apt_mask != PLACEHOLDER_IPv4_ADDR && (apt_ip.same_subnet(subnet_ip, apt_mask)));
        if ((select_auto && auto_cond) || (!select_auto && (exact_match_cond || subnet_cond)))
        {
            found = true;
            apt_info.name = std::string("\\Device\\NPF_") + pAdapter->AdapterName;
            apt_info.desc = pAdapter->Description;
            apt_info.ip = apt_ip;
            apt_info.mask = apt_mask;
            apt_info.gateway = apt_gateway;
            if (pAdapter->AddressLength != sizeof(eth_addr)) {
                LOG(WARNING) << "wrong address length: " << pAdapter->AddressLength;
            }
            else {
                auto c = reinterpret_cast<u_char*>(&apt_info.mac);
                for (unsigned i = 0; i < pAdapter->AddressLength; ++i) {
                    c[i] = pAdapter->Address[i];
                }
            }
            break;
        }
        pAdapter = pAdapter->Next;
    }
    if (select_auto && found) {
        VLOG(1) << "adapter selected automatically:\n" << apt_info << std::endl;
    }
    return apt_info;
}

adapter_info adapter_info::select_auto()
{
    return select_ip(PLACEHOLDER_IPv4_ADDR, false);
}

int packet_loop(
    pcap_t *adhandle,
    const std::chrono::system_clock::time_point &start_tm,
    int timeout_ms,
    const packet_loop_callback &cb)
{
    int res;
    pcap_pkthdr *pkthdr;
    const u_char *pkt_data;
    while((res = pcap_next_ex(adhandle, &pkthdr, &pkt_data)) >= 0)
    {
        if (timeout_ms > 0)
        {
            auto now_tm = std::chrono::system_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now_tm - start_tm);
            if (duration.count() >= timeout_ms) {
                VLOG(1) << "haven't got arp-reply after " << duration.count() << " ms";
                return NTLS_TIMEOUT_ERROR;
            }
        }
        if (res == 0) {
            VLOG(3) << "timeout elapsed";
            continue;
        }
        auto eh = reinterpret_cast<const ethernet_header*>(pkt_data);
        int rtn = cb(pkthdr, eh);
        if (rtn != NTLS_CONTINUE) {
            return rtn;
        }
    }
    if (res == -1) {
        throw std::runtime_error(fmt::format("failed to read packets: {}", pcap_geterr(adhandle)));
    }
    throw std::runtime_error("stop reading packets due to unexpected error");
}

int send_arp(
    pcap_t *adhandle,
    u_short op,
    const eth_addr &sea,
    const ip4_addr &sia,
    const eth_addr &dea,
    const ip4_addr &dia)
{
    arp_header ah;
    ah.h.d.dea = BROADCAST_ETH_ADDR;
    ah.h.d.sea = sea;
    ah.h.d.eth_type = htons(ETHERNET_TYPE_ARP);
    ah.d.hw_type = htons(ARP_HARDWARE_TYPE_ETHERNET);
    ah.d.proto = htons(ETHERNET_TYPE_IPv4);
    ah.d.hw_len = ETHERNET_ADDRESS_LEN;
    ah.d.proto_len = IPV4_ADDRESS_LEN;
    ah.d.op = htons(op);
    ah.d.sea = sea;
    ah.d.sia = sia;
    ah.d.dea = dea;
    ah.d.dia = dia;
    if (pcap_sendpacket(adhandle, reinterpret_cast<u_char*>(&ah), sizeof(arp_header)) != 0)
    {
        LOG(ERROR) << "failed to send packet: " << pcap_geterr(adhandle);
        return NTLS_FAILED;
    }
    return NTLS_SUCC;
}

int send_ip(
    pcap_t *adhandle,
    const eth_addr &dea,
    const eth_addr &sea,
    u_char proto,
    const ip4_addr &sia,
    const ip4_addr &dia,
    u_char ttl,
    void *ip_data,
    size_t len_in_byte,
    ip_header &ih_send)
{
    size_t total_len = sizeof(ip_header) + len_in_byte;
    u_char *packet = new u_char[total_len]{ 0 };
    std::shared_ptr<void*> packet_guard(nullptr, [=](void *){ delete[] packet; });
    auto ih = reinterpret_cast<ip_header*>(packet);
    ih->h.d.dea = dea;
    ih->h.d.sea = sea;
    ih->h.d.eth_type = htons(ETHERNET_TYPE_IPv4);
    ih->d.ver_ihl = (4 << 4) | 5;
    ih->d.tlen = htons(static_cast<u_short>(20 + len_in_byte));
    ih->d.id = rand_ushort();
    ih->d.ttl = ttl;
    ih->d.proto = proto;
    ih->d.sia = sia;
    ih->d.dia = dia;
    ih->d.crc = calc_checksum(&ih->d, sizeof(_ip_header_detail));
    memcpy(packet + sizeof(ip_header), ip_data, len_in_byte);
    if (pcap_sendpacket(adhandle, packet, static_cast<int>(total_len)) != 0)
    {
        LOG(ERROR) << "failed to send packet: " << pcap_geterr(adhandle);
        return NTLS_FAILED;
    }
    ih_send = *ih;
    return NTLS_SUCC;
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

std::ostream &operator<<(std::ostream &out, const pcap_if_t *dev)
{
    out << dev->name << std::endl;
    bool get_detail = false;
    adapter_info apt_info;
    for (pcap_addr_t *a = dev->addresses; a; a = a->next) {
        if (a->addr && a->addr->sa_family == AF_INET) {
            auto waddr = reinterpret_cast<const sockaddr_in*>(a->addr)->sin_addr;
            apt_info = adapter_info::select_ip(waddr, true);
            if (apt_info.ip == waddr) {
                get_detail = true;
                out << "\tDescription: " << apt_info.desc << std::endl;
                out << "\tMAC: " << apt_info.mac << std::endl;
            }
            break;
        }
    }
    if (!get_detail) {
        out << "\tDescription: " << dev->description << std::endl;
    }
    out << "\tLoopback: " << ((dev->flags & PCAP_IF_LOOPBACK) ? "yes" : "no") << std::endl;
    for (pcap_addr_t *a = dev->addresses; a; a = a->next) {
        out << "\t---------------------------------------" << std::endl;
        if (a->addr) {
            out << "\tAddress: " << a->addr << std::endl;
            auto waddr = reinterpret_cast<const sockaddr_in*>(a->addr)->sin_addr;
            if (get_detail && apt_info.ip == waddr &&
                apt_info.gateway != PLACEHOLDER_IPv4_ADDR)
            {
                out << "\tGateway: " << apt_info.gateway << std::endl;
            }
        }
        if (a->netmask) out << "\tNetmask: " << a->netmask << std::endl;
        if (a->broadaddr) out << "\tBroadcast: " << a->broadaddr << std::endl;
        if (a->dstaddr) out << "\tDestination: " << a->dstaddr << std::endl;
    }
    return out;
}

std::ostream &print_ip_packet(std::ostream &out, const ip_header *ih)
{
    switch (ih->d.proto)
    {
    case IPv4_TYPE_ICMP:
        out << *reinterpret_cast<const icmp_header*>(ih);
        break;
    case IPv4_TYPE_UDP:
        out << *reinterpret_cast<const udp_header*>(ih);
        break;
    default:
        out << *ih;
        break;
    }
    return out;
}

std::ostream &print_packet(
    std::ostream &out,
    const pcap_pkthdr *pkthdr,
    const ethernet_header *eh)
{
    time_t local_tv_sec = pkthdr->ts.tv_sec;
    tm ltime;
    localtime_s(&ltime, &local_tv_sec);
    char timestr[16];
    strftime(timestr, sizeof(timestr), "%H:%M:%S", &ltime);
    out << timestr << "." << std::setw(6) << std::left << pkthdr->ts.tv_usec << " ";
    out << eh->d.sea << " > " << eh->d.dea << std::endl;
    switch (ntohs(eh->d.eth_type))
    {
    case ETHERNET_TYPE_IPv4:
        print_ip_packet(out, reinterpret_cast<const ip_header*>(eh));
        break;
    case ETHERNET_TYPE_ARP:
    case ETHERNET_TYPE_RARP:
        out << *reinterpret_cast<const arp_header*>(eh);
        break;
    default:
        out << *eh;
        break;
    }
    return out;
}

int ip2mac(
    pcap_t *adhandle,
    const adapter_info &apt_info,
    const ip4_addr &ip,
    eth_addr &mac,
    int timeout_ms,
    bool use_cache)
{
    auto start_tm = std::chrono::system_clock::now();
    static std::map<ip4_addr, ip_mac_cache> ip_mac_map;
    if (use_cache && ip_mac_map.count(ip) > 0) {
        auto passed_sec = std::chrono::duration_cast<std::chrono::seconds>(start_tm - ip_mac_map[ip].tm);
        if (passed_sec.count() < 30) {
            VLOG(1) << "use cached mac for " << ip;
            mac = ip_mac_map[ip].mac;
            return NTLS_SUCC;
        }
        else {
            VLOG(1) << "cached mac for " << ip << " expired, send arp to update";
        }
    }
    std::atomic<bool> over = false;
    std::thread send_loop_t([&]{
        while (!over) {
            send_arp(adhandle, ARP_REQUEST_OP, apt_info.mac, apt_info.ip, PLACEHOLDER_ETH_ADDR, ip);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    });
    int rtn = packet_loop(adhandle, start_tm, timeout_ms, [&](pcap_pkthdr *pkthdr, const ethernet_header *eh){
        if (ntohs(eh->d.eth_type) == ETHERNET_TYPE_ARP) {
            auto ah = reinterpret_cast<const arp_header*>(eh);
            if (ntohs(ah->d.op) == ARP_REPLY_OP) {
                if (!ah->d.is_fake() && ah->d.sia == ip) {
                    mac = ah->d.sea;
                    ip_mac_cache cache;
                    cache.ip = ip;
                    cache.mac = mac;
                    cache.tm = std::chrono::system_clock::now();
                    ip_mac_map[ip] = cache;
                    return NTLS_SUCC;
                }
            }
        }
        return NTLS_CONTINUE;
    });
    over = true;
    send_loop_t.join();
    return rtn;
}

void get_time_of_day(timeval *tv)
{
    SYSTEMTIME sys_tm;
    GetLocalTime(&sys_tm);
    tm tm;
    tm.tm_year   = sys_tm.wYear - 1900;
    tm.tm_mon   = sys_tm.wMonth - 1;
    tm.tm_mday   = sys_tm.wDay;
    tm.tm_hour   = sys_tm.wHour;
    tm.tm_min   = sys_tm.wMinute;
    tm.tm_sec   = sys_tm.wSecond;
    tm. tm_isdst  = -1;
    time_t clock = mktime(&tm);
    tv->tv_sec = static_cast<long>(clock);
    tv->tv_usec = sys_tm.wMilliseconds * 1000;
}

long operator-(const timeval &tv1, const timeval &tv2)
{
    long diff_sec = tv1.tv_sec - tv2.tv_sec;
    long diff_ms = (tv1.tv_usec - tv2.tv_usec) / 1000;
    return (diff_sec * 1000 + diff_ms);
}

int ping(
    pcap_t *adhandle,
    const adapter_info &apt_info,
    const ip4_addr &target_ip,
    u_char ttl,
    int timeout_ms,
    long &cost_ms,
    ip_header &ih_recv,
    _icmp_error_detail &d_err)
{
    auto start_tm = std::chrono::system_clock::now();
    _icmp_header_detail ping_data { 0 };
    ping_data.type = ICMP_TYPE_PING_ASK;
    ping_data.code = 0;
    ping_data.id = rand_ushort();
    ping_data.sn = rand_ushort();
    ping_data.crc = calc_checksum(&ping_data, sizeof(_icmp_header_detail));
    eth_addr dest_mac = BROADCAST_ETH_ADDR;
    bool is_local = target_ip.same_subnet(apt_info.ip, apt_info.mask);
    if (!is_local) {
        VLOG(1) << "nonlocal target ip, send icmp to gateway instread of broadcasting";
        if (ip2mac(adhandle, apt_info, apt_info.gateway, dest_mac, 5000) != NTLS_SUCC) {
            throw std::runtime_error(fmt::format("can't resolve mac address of gateway {}", to_string(apt_info.gateway)));
        }
    }
    timeval send_tv;
    get_time_of_day(&send_tv);
    ip_header ih_send;
    if (send_ip(adhandle, dest_mac, apt_info.mac, IPv4_TYPE_ICMP, apt_info.ip, target_ip, ttl,
        &ping_data, sizeof(_icmp_header_detail), ih_send) != NTLS_SUCC)
    {
        throw std::runtime_error("failed to send ipv4 packet");
    }

    int rtn = packet_loop(adhandle, start_tm, timeout_ms, [&](pcap_pkthdr *pkthdr, const ethernet_header *eh){
        if (eh->d.dea == apt_info.mac && ntohs(eh->d.eth_type) == ETHERNET_TYPE_IPv4) {
            auto ih = reinterpret_cast<const ip_header*>(eh);
            if (ih->d.dia == apt_info.ip && ih->d.proto == IPv4_TYPE_ICMP) {
                auto mh = reinterpret_cast<const icmp_header*>(eh);
                if (mh->d.type == ICMP_TYPE_PING_REPLY && mh->d.id == ping_data.id && mh->d.sn == ping_data.sn) {
                    VLOG_IF(1, is_local) << target_ip << " is at " << eh->d.sea;
                    ih_recv = mh->h;
                    cost_ms = pkthdr->ts - send_tv;
                    return NTLS_SUCC;
                }
                if (mh->d.is_typeof_error()) {
                    auto mh = reinterpret_cast<const _icmp_error_detail*>(eh);
                    if (mh->e_ip.almost_same(ih_send.d)) {
                        d_err = *mh;
                        cost_ms = pkthdr->ts - send_tv;
                        return NTLS_RECV_ERROR_ICMP;
                    }
                }
            }
        }
        return NTLS_CONTINUE;
    });
    return rtn;
}
