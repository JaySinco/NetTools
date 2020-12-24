#include "packet-listctrl.h"
#include "net/ethernet.h"
#include "net/ipv4.h"
#include "net/udp.h"
#include "net/tcp.h"

wxString PacketListCtrl::OnGetItemText(long item, long column) const
{
    return stringfy_field(data_ptr->at(idx_ptr->at(item)), column);
}

wxListItemAttr *PacketListCtrl::OnGetItemAttr(long item) const
{
    wxListItemAttr attr;
    const auto &layers = data_ptr->at(idx_ptr->at(item)).get_detail().layers;
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

void PacketListCtrl::init(const std::vector<size_t> *p_idx, const std::vector<packet> *p_data)
{
    idx_ptr = p_idx;
    data_ptr = p_data;
    AppendColumn("time", wxLIST_FORMAT_LEFT, 105);
    AppendColumn("sip", wxLIST_FORMAT_LEFT, 120);
    AppendColumn("dip", wxLIST_FORMAT_LEFT, 120);
    AppendColumn("sport", wxLIST_FORMAT_LEFT, 55);
    AppendColumn("dport", wxLIST_FORMAT_LEFT, 55);
    AppendColumn("type", wxLIST_FORMAT_LEFT, 70);
    AppendColumn("owner", wxLIST_FORMAT_LEFT, 120);
    SetItemCount(0);
}

void PacketListCtrl::clear()
{
    SetItemCount(0);
    attr_list.clear();
    attr_list.shrink_to_fit();
}

std::string PacketListCtrl::stringfy_field(const packet &pac, long column)
{
    const auto &j = pac.to_json();
    const auto &layers = j["layers"];
    switch (column) {
        case FIELD_TIME:
            return j["time"];
        case FIELD_SOURCE_IP:
            if (layers.size() > 1 && layers[1] == Protocol_Type_IPv4) {
                return j[Protocol_Type_IPv4]["source-ip"];
            }
            break;
        case FIELD_DEST_IP:
            if (layers.size() > 1 && layers[1] == Protocol_Type_IPv4) {
                return j[Protocol_Type_IPv4]["dest-ip"];
            }
            break;
        case FIELD_SOURCE_PORT:
            if (layers.size() > 2 && layers[2] == Protocol_Type_UDP) {
                return std::to_string(j[Protocol_Type_UDP]["source-port"].get<u_short>());
            }
            if (layers.size() > 2 && layers[2] == Protocol_Type_TCP) {
                return std::to_string(j[Protocol_Type_TCP]["source-port"].get<u_short>());
            }
            break;
        case FIELD_DEST_PORT:
            if (layers.size() > 2 && layers[2] == Protocol_Type_UDP) {
                return std::to_string(j[Protocol_Type_UDP]["dest-port"].get<u_short>());
            }
            if (layers.size() > 2 && layers[2] == Protocol_Type_TCP) {
                return std::to_string(j[Protocol_Type_TCP]["dest-port"].get<u_short>());
            }
            break;
        case FIELD_TYPE: {
            auto &p_back = pac.get_detail().layers.back();
            std::string type = p_back->succ_type();
            if (!protocol::is_specific(type)) {
                type = p_back->type();
            }
            return type;
        }
        case FIELD_OWNER:
            return j["owner"];
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
