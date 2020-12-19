#include "packet-listctrl.h"
#include "net/ethernet.h"
#include "net/ipv4.h"
#include "net/udp.h"

wxString PacketListCtrl::OnGetItemText(long item, long column) const
{
    return stringfy_field(data_ptr->at(item), column);
}

wxListItemAttr *PacketListCtrl::OnGetItemAttr(long item) const
{
    wxListItemAttr attr;
    const auto &layers = data_ptr->at(item).get_detail().layers;
    if (layers.size() > 1 && layers[1]->type() == Protocol_Type_IPv4) {
        const auto &ih = dynamic_cast<const ipv4 &>(*layers[1]);
        const ip4 &sip = ih.get_detail().sip;
        const ip4 &dip = ih.get_detail().dip;
        attr.SetTextColour(hashed_color(sip, dip));
    } else {
        attr.SetTextColour(wxColour(211, 211, 211));
    }
    auto attr_it = std::find(attr_list.cbegin(), attr_list.cend(), attr);
    if (attr_it != attr_list.cend()) {
        return const_cast<wxListItemAttr *>(&*attr_it);
    }
    const_cast<std::vector<wxListItemAttr> &>(attr_list).push_back(attr);
    return const_cast<wxListItemAttr *>(&attr_list.back());
}

void PacketListCtrl::init(const std::vector<packet> *ptr)
{
    data_ptr = ptr;
    AppendColumn("time", wxLIST_FORMAT_LEFT, 105);
    AppendColumn("smac", wxLIST_FORMAT_LEFT, 140);
    AppendColumn("dmac", wxLIST_FORMAT_LEFT, 140);
    AppendColumn("sip", wxLIST_FORMAT_LEFT, 120);
    AppendColumn("dip", wxLIST_FORMAT_LEFT, 120);
    AppendColumn("sport", wxLIST_FORMAT_LEFT, 55);
    AppendColumn("dport", wxLIST_FORMAT_LEFT, 55);
    AppendColumn("type", wxLIST_FORMAT_LEFT, 70);
    AppendColumn("owner", wxLIST_FORMAT_LEFT, 80);
    SetItemCount(0);
}

void PacketListCtrl::clear() { attr_list.clear(); }

std::string PacketListCtrl::stringfy_field(const packet &pac, long column)
{
    const auto &layers = pac.get_detail().layers;
    switch (column) {
        case FIELD_TIME:
            return util::tv2s(pac.get_detail().time);
        case FIELD_SOURCE_MAC:
            if (layers.front()->type() == Protocol_Type_Ethernet) {
                const auto &eh = dynamic_cast<const ethernet &>(*layers.front());
                return eh.get_detail().smac.to_str();
            }
            break;
        case FIELD_DEST_MAC:
            if (layers.front()->type() == Protocol_Type_Ethernet) {
                const auto &eh = dynamic_cast<const ethernet &>(*layers.front());
                return eh.get_detail().dmac.to_str();
            }
            break;
        case FIELD_SOURCE_IP:
            if (layers.size() > 1 && layers[1]->type() == Protocol_Type_IPv4) {
                const auto &ih = dynamic_cast<const ipv4 &>(*layers[1]);
                return ih.get_detail().sip.to_str();
            }
            break;
        case FIELD_DEST_IP:
            if (layers.size() > 1 && layers[1]->type() == Protocol_Type_IPv4) {
                const auto &ih = dynamic_cast<const ipv4 &>(*layers[1]);
                return ih.get_detail().dip.to_str();
            }
            break;
        case FIELD_SOURCE_PORT:
            if (layers.size() > 2 && layers[2]->type() == Protocol_Type_UDP) {
                const auto &uh = dynamic_cast<const udp &>(*layers[2]);
                return std::to_string(uh.get_detail().sport);
            }
            break;
        case FIELD_DEST_PORT:
            if (layers.size() > 2 && layers[2]->type() == Protocol_Type_UDP) {
                const auto &uh = dynamic_cast<const udp &>(*layers[2]);
                return std::to_string(uh.get_detail().dport);
            }
            break;
        case FIELD_TYPE: {
            std::string type = layers.back()->succ_type();
            if (!protocol::is_specific(type)) {
                type = layers.back()->type();
            }
            return type;
        }
        case FIELD_OWNER:
            return pac.get_detail().owner;
        default:
            throw std::runtime_error("invalid column {}"_format(column));
    }
    return "";
}

wxColour PacketListCtrl::hashed_color(const std::string &data)
{
    std::hash<std::string> hash_func;
    size_t hash = hash_func(data);
    auto p = reinterpret_cast<unsigned char *>(&hash);
    return wxColour(p[0], p[1], p[2]);
}

wxColour PacketListCtrl::hashed_color(ip4 a, ip4 b)
{
    if (a > b) {
        std::swap(a, b);
    }
    char buf[sizeof(ip4) * 2];
    std::memcpy(buf, &a, sizeof(ip4));
    std::memcpy(buf + sizeof(ip4), &b, sizeof(ip4));
    return hashed_color(std::string(buf, sizeof(buf)));
}
