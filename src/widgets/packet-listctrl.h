#pragma once
#include "transport/packet.h"
#include <wx/listctrl.h>

class PacketListCtrl : public wxListCtrl
{
public:
    enum LIST_IDX
    {
        FIELD_TIME,
        FIELD_SOURCE_IP,
        FIELD_DEST_IP,
        FIELD_SOURCE_PORT,
        FIELD_DEST_PORT,
        FIELD_TYPE,
        FIELD_OWNER,
        __FIELD_SIZE__,
    };

    using wxListCtrl::wxListCtrl;

    wxString OnGetItemText(long item, long column) const override;
    wxListItemAttr *OnGetItemAttr(long item) const override;

    void init(const std::vector<size_t> *p_idx, const std::vector<packet> *p_data);
    void clear();

    static std::string stringfy_field(const packet &pac, long column);

private:
    const std::vector<size_t> *idx_ptr = nullptr;
    const std::vector<packet> *data_ptr = nullptr;
    std::vector<wxListItemAttr> attr_list;

    static wxColour hashed_color(ip4 a, ip4 b);
    static wxColour hashed_color(const std::string &data);
};
