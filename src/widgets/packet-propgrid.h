#pragma once
#include "transport/packet.h"
#include <wx/propgrid/propgrid.h>

class PacketPropGrid : public wxPropertyGrid
{
public:
    using wxPropertyGrid::wxPropertyGrid;

    void show_packet(const packet &pac);
    void show_json(wxPGProperty *parent, const std::string &name, const json &j);
};
