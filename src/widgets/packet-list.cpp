#include "packet-list.h"
#include "core/ethernet.h"
#include "core/ipv4.h"
#include "core/udp.h"

PacketList::PacketList(wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size,
                       long style, const wxValidator &validator, const wxString &name)
    : wxListCtrl(parent, id, pos, size, style, validator, name)
{
}

wxString PacketList::OnGetItemText(long item, long column) const
{
    return stringfy_field(pac_list_ptr->at(item), column);
}

wxListItemAttr *PacketList::OnGetItemAttr(long item) const
{
    wxListItemAttr attr;
    const auto &layers = pac_list_ptr->at(item).get_detail().layers;
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

void PacketList::SetPacPtr(const std::vector<packet> *ptr) { pac_list_ptr = ptr; }

void PacketList::CleanBuf() { attr_list.clear(); }

std::string PacketList::stringfy_field(const packet &pac, long column)
{
    const auto &layers = pac.get_detail().layers;
    switch (column) {
        case FIELD_TIME:
            return tv2s(pac.get_detail().time);
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
        default:
            throw std::runtime_error(fmt::format("invalid column {}", column));
    }
    return "";
}
