#pragma once
#include "type.h"
#include "core/transport.h"

class PacketProp : public wxPropertyGrid
{
public:
    using wxPropertyGrid::wxPropertyGrid;

    void show_packet(const packet &pac);
    void show_json(wxPGProperty *parent, const std::string &name, const json &j);
};
