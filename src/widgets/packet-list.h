#pragma once
#include "type.h"
#include "core/transport.h"

class PacketList : public wxListCtrl
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

    PacketList(wxWindow *parent, wxWindowID id = wxID_ANY, const wxPoint &pos = wxDefaultPosition,
               const wxSize &size = wxDefaultSize, long style = wxLC_ICON,
               const wxValidator &validator = wxDefaultValidator,
               const wxString &name = "PacketList");

    wxString OnGetItemText(long item, long column) const override;
    wxListItemAttr *OnGetItemAttr(long item) const override;

    void SetPacPtr(const std::vector<packet> *ptr);
    void CleanBuf();

    static std::string stringfy_field(const packet &pac, long column);

private:
    const std::vector<packet> *pac_list_ptr = nullptr;
    std::vector<wxListItemAttr> attr_list;
};
