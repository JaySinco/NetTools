#pragma once
#include "type.h"
#include "core/transport.h"

class PacketProp : public wxPropertyGrid
{
public:
    PacketProp(wxWindow *parent, wxWindowID id = wxID_ANY, const wxPoint &pos = wxDefaultPosition,
               const wxSize &size = wxDefaultSize, long style = wxLC_ICON,
               const wxString &name = "PacketProp");

    void show_packet(const packet &pac);
    void show_json(wxPGProperty *parent, const std::string &name, const json &j);
};
