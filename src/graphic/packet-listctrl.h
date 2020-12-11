#pragma once
#include "prec.h"
#include <wx/listctrl.h>
#include "net/packet.h"

class PacketListCtrl : public wxListCtrl
{
public:
    enum LIST_IDX
    {
        FIELD_TIME,
        FIELD_SOURCE_MAC,
        FIELD_DEST_MAC,
        FIELD_SOURCE_IP,
        FIELD_DEST_IP,
        FIELD_SOURCE_PORT,
        FIELD_DEST_PORT,
        FIELD_TYPE,
        __FIELD_SIZE__,
    };

    using wxListCtrl::wxListCtrl;

    wxString OnGetItemText(long item, long column) const override;
    wxListItemAttr *OnGetItemAttr(long item) const override;

    void init(const std::vector<packet> *ptr);
    void clear();

    static std::string stringfy_field(const packet &pac, long column);

private:
    const std::vector<packet> *data_ptr = nullptr;
    std::vector<wxListItemAttr> attr_list;

    static wxColour hashed_color(ip4 a, ip4 b);
    static wxColour hashed_color(const std::string &data);
};
