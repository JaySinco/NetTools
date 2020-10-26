#include "packet.h"
#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <gflags/gflags.h>
#include <glog/logging.h>
#include "ethernet.h"
#include "arp.h"

packet::packet(const u_char *const start, const u_char *const end)
{
    const u_char *pstart = start;
    std::string type = Protocol_Type_Ethernet;
    while (type != Protocol_Type_Void) {
        const u_char *pend;
        std::shared_ptr<protocol> prot;
        if (type == Protocol_Type_Ethernet) {
            prot = std::make_shared<ethernet>(pstart, pend);
        } else if (type == Protocol_Type_ARP || type == Protocol_Type_RARP) {
            prot = std::make_shared<arp>(pstart, pend);
        } else {
            VLOG(1) << "unexpected protocol type after " << stack.back()->type() << ": " << type;
            break;
        }
        stack.push_back(prot);
        pstart = pend;
        type = prot->succ_type();
    }
}

void packet::push_front(std::shared_ptr<protocol> prot)
{
    auto succ_type = prot->succ_type();
    auto type = stack.front()->type();
    if (succ_type != type) {
        throw std::runtime_error(
            fmt::format("protocol type mismatch: {} -> {} | {}", prot->type(), succ_type, type));
    }
    stack.insert(stack.begin(), prot);
}

void packet::to_bytes(std::vector<u_char> &bytes) const
{
    for (auto it = stack.crbegin(); it != stack.crend(); ++it) {
        (*it)->to_bytes(bytes);
    }
}
