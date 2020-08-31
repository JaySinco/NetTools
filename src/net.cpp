#include "net.h"
#include <thread>
#include <atomic>
#include <random>
#include <iphlpapi.h>

u_short rand_ushort()
{
    static std::random_device rd;
    static std::default_random_engine engine(rd());
    static std::uniform_int_distribution<u_short> dist;
    return dist(engine);
}

pcap_t *open_target_adaptor(const ip4_addr &ip, bool exact_match, adapter_info &apt_info)
{
    apt_info = adapter_info(ip, exact_match);
    if (apt_info.name.size() == 0) {
        throw std::runtime_error(nt::sout << "no adapter found in same local network with " << ip);
    }
    pcap_t *adhandle;
    char errbuf[PCAP_ERRBUF_SIZE];
    if (!(adhandle= pcap_open(apt_info.name.c_str(), 65536, PCAP_OPENFLAG_PROMISCUOUS, 1000, NULL, errbuf)))
    {
        throw std::runtime_error(nt::sout << "failed to open the adapter: " << apt_info.name);
    }
    return adhandle;
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
            (!exact_match && apt_mask != PLACEHOLDER_IPv4_ADDR && 
                (apt_ip & apt_mask) == (subnet_ip & apt_mask)))
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

bool ip2mac(
    pcap_t *adhandle,
    const adapter_info &apt_info,
    const ip4_addr &ip,
    eth_addr &mac,
    int timeout_ms)
{
    auto start_tm = std::chrono::system_clock::now();
    std::atomic<bool> over = false;
    std::thread send_loop_t([&]{
        while (!over) {
            send_arp(adhandle, ARP_REQUEST_OP, apt_info.mac, apt_info.ip, PLACEHOLDER_ETH_ADDR, ip);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    });
    
    int res;
    pcap_pkthdr *header;
    const u_char *pkt_data;
    bool succ = false;
    while((res = pcap_next_ex(adhandle, &header, &pkt_data)) >= 0)
    {
        if (timeout_ms > 0)
        {
            auto now_tm = std::chrono::system_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now_tm - start_tm);
            if (duration.count() >= timeout_ms) {
                LOG(ERROR) << "haven't got arp reply after " << duration.count() << " ms";
                break;
            }

        }
        if (res == 0) {
            VLOG(3) << "timeout elapsed";
            continue;
        }
        auto eh = reinterpret_cast<const ethernet_header*>(pkt_data);
        if (ntohs(eh->eth_type) == ETHERNET_TYPE_ARP) {
            auto ah = reinterpret_cast<const eth_ip4_arp*>(pkt_data + sizeof(ethernet_header));
            if (ntohs(ah->op) == ARP_REPLY_OP) {
                if (!ah->fake() && ah->sia == ip) {
                    mac = ah->sea;
                    succ = true;
                    break;
                }
            }
        }
    }
    if (res == -1) {
        LOG(ERROR) << "failed to read packets: " << pcap_geterr(adhandle);
    }
    over = true;
    send_loop_t.join();
    return succ;
}

bool is_reachable(pcap_t *adhandle, const adapter_info &apt_info, const ip4_addr &target_ip, int timeout_ms)
{
    auto start_tm = std::chrono::system_clock::now();
    icmp_ping icmp_data { 0 };
    icmp_data.type = ICMP_TYPE_PING_ASK;
    icmp_data.code = 0;
    icmp_data.id = rand_ushort();
    icmp_data.sn = rand_ushort();
    icmp_data.crc = calc_checksum(&icmp_data, sizeof(icmp_ping));
    bool ok = send_ip4(adhandle, BROADCAST_ETH_ADDR, apt_info.mac, IPv4_TYPE_ICMP,
        apt_info.ip, target_ip, &icmp_data, sizeof(icmp_ping));
    if (!ok) {
        return false;
    }
    
    int res;
    pcap_pkthdr *header;
    const u_char *pkt_data;
    bool succ = false;
    while((res = pcap_next_ex(adhandle, &header, &pkt_data)) >= 0)
    {
        if (timeout_ms > 0)
        {
            auto now_tm = std::chrono::system_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now_tm - start_tm);
            if (duration.count() >= timeout_ms) {
                return false;
            }

        }
        if (res == 0) {
            VLOG(3) << "timeout elapsed";
            continue;
        }
        auto eh = reinterpret_cast<const ethernet_header*>(pkt_data);
        if (eh->dea == apt_info.mac && ntohs(eh->eth_type) == ETHERNET_TYPE_IPv4) {
            auto ih = reinterpret_cast<const ip4_header*>(pkt_data + sizeof(ethernet_header));
            if (ih->dia == apt_info.ip && ih->proto == IPv4_TYPE_ICMP) {
                auto mh = reinterpret_cast<const icmp_ping*>(pkt_data + sizeof(ethernet_header) + sizeof(ip4_header));
                if (mh->type == ICMP_TYPE_PING_REPLY && mh->id == icmp_data.id && mh->sn == icmp_data.sn) {
                    VLOG(1) << target_ip << " is at " << eh->sea;
                    return true;
                }
            }
        }
    }
    if (res == -1) {
        LOG(ERROR) << "failed to read packets: " << pcap_geterr(adhandle);
    }
    return false;
}

bool send_arp(
    pcap_t *adhandle,
    u_short op,
    const eth_addr &sea,
    const ip4_addr &sia,
    const eth_addr &dea,
    const ip4_addr &dia)
{
    u_char packet[sizeof(ethernet_header) + sizeof(eth_ip4_arp)] = { 0 };
    auto eh = reinterpret_cast<ethernet_header*>(packet);
    eh->dea = BROADCAST_ETH_ADDR;
    eh->sea = sea;
    eh->eth_type = htons(ETHERNET_TYPE_ARP);
    auto ah = reinterpret_cast<eth_ip4_arp*>(packet + sizeof(ethernet_header));
    ah->hw_type = htons(ARP_HARDWARE_TYPE_ETHERNET);
    ah->proto = htons(ETHERNET_TYPE_IPv4);
    ah->hw_len = ETHERNET_ADDRESS_LEN;
    ah->proto_len = IPV4_ADDRESS_LEN;
    ah->op = htons(op);
    ah->sea = sea;
    ah->sia = sia;
    ah->dea = dea;
    ah->dia = dia;
    if (pcap_sendpacket(adhandle, packet, sizeof(packet)/sizeof(packet[0])) != 0)
    {
        LOG(ERROR) << "failed to send packet: " << pcap_geterr(adhandle);
        return false;
    }
    return true;
}

bool send_ip4(
    pcap_t *adhandle,
    const eth_addr &dea,
    const eth_addr &sea,
    u_char proto,
    const ip4_addr &sia,
    const ip4_addr &dia,
    void *ip_data,
    size_t len_in_byte)
{
    size_t total_len = sizeof(ethernet_header) + sizeof(ip4_header) + len_in_byte;
    u_char *packet = new u_char[total_len]{ 0 };
    std::shared_ptr<void*> packet_guard(nullptr, [=](void *){ delete[] packet; });
    auto eh = reinterpret_cast<ethernet_header*>(packet);
    eh->dea = dea;
    eh->sea = sea;
    eh->eth_type = htons(ETHERNET_TYPE_IPv4);
    auto ih = reinterpret_cast<ip4_header*>(packet + sizeof(ethernet_header));
    ih->ver_ihl = (4 << 4) | 5;
    ih->tlen = htons(static_cast<u_short>(20 + len_in_byte));
    std::uniform_int_distribution<u_short> us_dist;
    ih->id = rand_ushort();
    ih->ttl = 128;
    ih->proto = proto;
    ih->sia = sia;
    ih->dia = dia;
    ih->crc = calc_checksum(ih, sizeof(ip4_header));
    memcpy(packet + sizeof(ethernet_header) + sizeof(ip4_header), ip_data, len_in_byte);
    if (pcap_sendpacket(adhandle, packet, static_cast<int>(total_len)) != 0)
    {
        LOG(ERROR) << "failed to send packet: " << pcap_geterr(adhandle);
        return false;
    }
    return true;
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
            apt_info = adapter_info(waddr, true);
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

std::ostream &print_packet(
    std::ostream &out,
    const pcap_pkthdr *header,
    const u_char *pkt_data)
{
    time_t local_tv_sec = header->ts.tv_sec;
    tm ltime;
    localtime_s(&ltime, &local_tv_sec);
    char timestr[16];
    strftime(timestr, sizeof(timestr), "%H:%M:%S", &ltime);
    out << timestr << "." << std::setw(6) << std::left << header->ts.tv_usec << " ";
    auto eh = reinterpret_cast<const ethernet_header*>(pkt_data);
    out << eh->sea << " > " << eh->dea << std::endl;
    u_short ethtyp = ntohs(eh->eth_type);
    const u_char *ptr = pkt_data + sizeof(ethernet_header);
    switch (ethtyp)
    {
    case ETHERNET_TYPE_IPv4:
    {
        auto ih = reinterpret_cast<const ip4_header*>(ptr);
        out << ih;
        break;
    }
    case ETHERNET_TYPE_IPv6:
        out << "\tEthernet type: IPv6\n";
        break;
    case ETHERNET_TYPE_ARP:
    case ETHERNET_TYPE_RARP:
    {
        auto ah = reinterpret_cast<const eth_ip4_arp*>(ptr);
        out << ah;
        break;
    }
    default:
        LOG(ERROR) << "unknow ethernet type: 0x" << std::hex << ethtyp << std::dec;
    }
    return out;
}