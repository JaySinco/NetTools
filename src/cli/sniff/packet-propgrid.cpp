#include "packet-propgrid.h"

void PacketPropGrid::show_packet(const packet &pac)
{
    Clear();
    const auto &j = pac.to_json();
    const auto &layers = j["layers"];
    for (auto it = layers.rbegin(); it != layers.rend(); ++it) {
        std::string type = *it;
        Append(new wxPropertyCategory(type, wxPG_LABEL));
        const auto &value = j[type];
        for (auto vit = value.begin(); vit != value.end(); ++vit) {
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
    } else {
        auto p = new wxStringProperty(name, wxPG_LABEL, j.dump());
        if (parent != nullptr) {
            AppendIn(parent, p);
        } else {
            Append(p);
        }
    }
}
