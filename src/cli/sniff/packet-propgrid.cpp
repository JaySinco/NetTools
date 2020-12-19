#include "packet-propgrid.h"

void PacketPropGrid::show_packet(const packet &pac)
{
    Clear();
    json layers = pac.to_json()["layers"];
    for (auto it = layers.rbegin(); it != layers.rend(); ++it) {
        std::string type = (*it)["type"].get<std::string>();
        Append(new wxPropertyCategory(type, wxPG_LABEL));
        for (auto vit = it->begin(); vit != it->end(); ++vit) {
            show_json(nullptr, vit.key(), vit.value());
        }
    }
    Refresh();
}

void PacketPropGrid::show_json(wxPGProperty *parent, const std::string &name, const json &j)
{
    if (j.is_array()) {
        wxPGProperty *p = nullptr;
        wxStringProperty *sp = new wxStringProperty(name, wxPG_LABEL, "<composed>");
        if (parent != nullptr) {
            p = AppendIn(parent, sp);
        } else {
            p = Append(sp);
        }
        int index = 0;
        for (auto it = j.begin(); it != j.end(); ++it) {
            show_json(p, std::to_string(index), *it);
            ++index;
        }
    } else if (j.is_object()) {
        wxPGProperty *p = nullptr;
        wxStringProperty *sp = new wxStringProperty(name, wxPG_LABEL, "<composed>");
        if (parent != nullptr) {
            p = AppendIn(parent, sp);
        } else {
            p = Append(sp);
        }
        for (auto it = j.begin(); it != j.end(); ++it) {
            show_json(p, it.key(), it.value());
        }
    } else if (j.is_string()) {
        auto p = new wxStringProperty(name, wxPG_LABEL, j.get<std::string>());
        if (parent != nullptr) {
            AppendIn(parent, p);
        } else {
            Append(p);
        }
    } else if (j.is_number()) {
        auto p = new wxStringProperty(name, wxPG_LABEL, std::to_string(j.get<long long>()));
        if (parent != nullptr) {
            AppendIn(parent, p);
        } else {
            Append(p);
        }
    } else if (j.is_boolean()) {
        auto p = new wxBoolProperty(name, wxPG_LABEL, j.get<bool>());
        if (parent != nullptr) {
            AppendIn(parent, p);
        } else {
            Append(p);
        }
    } else {
        auto p = new wxStringProperty(name, wxPG_LABEL, j.dump());
        if (parent != nullptr) {
            AppendIn(parent, p);
        } else {
            Append(p);
        }
    }
}
