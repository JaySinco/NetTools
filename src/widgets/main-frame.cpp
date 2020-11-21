#include "main-frame.h"
#include <thread>

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
};

MainFrame::MainFrame(const wxPoint &pos, const wxSize &size)
    : wxFrame(NULL, wxID_ANY, "NetTools", pos, size)
{
    setup_ui();
    auto &apt_def = adaptor::fit();
    int apt_idx = 0;
    for (const auto &apt : adaptor::all()) {
        if (apt_def.name != apt.name) {
            ++apt_idx;
        }
        m_adaptor->AppendString(apt.ip.to_str());
    }
    m_adaptor->SetSelection(apt_idx);
    m_stop->Disable();
    m_list->SetPacPtr(&pac_list);
    m_list->AppendColumn("time", wxLIST_FORMAT_LEFT, 105);
    m_list->AppendColumn("smac", wxLIST_FORMAT_LEFT, 140);
    m_list->AppendColumn("dmac", wxLIST_FORMAT_LEFT, 140);
    m_list->AppendColumn("sip", wxLIST_FORMAT_LEFT, 120);
    m_list->AppendColumn("dip", wxLIST_FORMAT_LEFT, 120);
    m_list->AppendColumn("sport", wxLIST_FORMAT_LEFT, 55);
    m_list->AppendColumn("dport", wxLIST_FORMAT_LEFT, 55);
    m_list->AppendColumn("type", wxLIST_FORMAT_LEFT, 70);
    m_list->SetItemCount(0);
    m_prop->SetSplitterPosition(180);
    column_sort.resize(PacketList::__FIELD_SIZE__, false);

    Bind(wxEVT_MENU, &MainFrame::on_quit, this, ID_QUIT);
    Bind(wxEVT_MENU, &MainFrame::on_about, this, ID_ABOUT);
    Bind(wxEVT_BUTTON, &MainFrame::on_sniff_start, this, ID_SNIFFSTART);
    Bind(wxEVT_BUTTON, &MainFrame::on_sniff_stop, this, ID_SNIFFSTOP);
    Bind(wxEVT_BUTTON, &MainFrame::on_sniff_clear, this, ID_SNIFFCLEAR);
    m_list->Bind(wxEVT_LIST_ITEM_SELECTED, &MainFrame::on_packet_selected, this);
    m_list->Bind(wxEVT_LIST_COL_CLICK, &MainFrame::on_list_col_clicked, this);
}

void MainFrame::on_quit(wxCommandEvent &event) { Close(true); }

void MainFrame::on_about(wxCommandEvent &event)
{
    wxAboutDialogInfo info;
    info.SetName("NetTools");
    info.SetDescription("Network Toolset Program");
    info.AddDeveloper("Mr.Robot");
    info.SetWebSite("https://github.com/JaySinco/NetTools");
    wxAboutBox(info, this);
}

void MainFrame::on_sniff_start(wxCommandEvent &event)
{
    m_start->Disable();
    m_stop->Enable();
    sniff_should_stop = false;
    const adaptor &apt = adaptor::all().at(m_adaptor->GetSelection());
    std::string filter = m_filter->GetValue();
    std::thread(&MainFrame::sniff_background, this, apt, filter, 500).detach();
}

void MainFrame::on_sniff_stop(wxCommandEvent &event) { sniff_should_stop = true; }

void MainFrame::on_sniff_clear(wxCommandEvent &event)
{
    m_list->DeleteAllItems();
    m_list->CleanBuf();
    m_prop->Clear();
    pac_list.clear();
}

void MainFrame::on_packet_selected(wxListEvent &event)
{
    m_prop->Clear();
    int idx = event.m_itemIndex;
    json layers = pac_list.at(idx).to_json()["layers"];
    for (auto it = layers.rbegin(); it != layers.rend(); ++it) {
        std::string type = (*it)["type"].get<std::string>();
        m_prop->Append(new wxPropertyCategory(type, wxPG_LABEL));
        for (auto vit = it->begin(); vit != it->end(); ++vit) {
            show_json_prop(nullptr, vit.key(), vit.value());
        }
    }
    m_prop->Refresh();
}

void MainFrame::on_list_col_clicked(wxListEvent &event)
{
    if (!m_start->IsEnabled()) {
        return;
    }
    bool reverse = column_sort.at(event.m_col);
    column_sort.at(event.m_col) = !reverse;
    std::stable_sort(pac_list.begin(), pac_list.end(), [&](const packet &a, const packet &b) {
        auto sa = PacketList::stringfy_field(a, event.m_col);
        auto sb = PacketList::stringfy_field(b, event.m_col);
        return reverse ? sb < sa : sa < sb;
    });
    m_list->Refresh();
}

void MainFrame::sniff_background(const adaptor &apt, const std::string &filter, int update_freq_ms)
{
    try {
        pcap_t *handle = transport::open_adaptor(apt, update_freq_ms);
        std::shared_ptr<void> handle_guard(nullptr, [&](void *) {
            pcap_close(handle);
            this->GetEventHandler()->CallAfter(std::bind(&MainFrame::sniff_stopped, this));
            LOG(INFO) << "sniff stopped";
        });
        if (filter.size() > 0) {
            LOG(INFO) << "set filter \"" << filter << "\", mask=" << apt.mask.to_str();
            transport::setfilter(handle, filter, apt.mask);
        }
        LOG(INFO) << "begin to sniff...";
        int res;
        pcap_pkthdr *info;
        const u_char *start;
        std::vector<packet> data;
        auto start_tm = std::chrono::system_clock::now();
        while ((res = pcap_next_ex(handle, &info, &start)) >= 0) {
            if (sniff_should_stop) {
                break;
            }
            auto now = std::chrono::system_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_tm);
            if (duration.count() >= update_freq_ms) {
                if (data.size() > 0) {
                    this->GetEventHandler()->CallAfter(
                        std::bind(&MainFrame::sniff_recv, this, std::move(data)));
                    data.clear();
                }
                start_tm = now;
            }
            if (res == 0) {
                continue;  // timeout elapsed
            }
            packet pac(start, start + info->len, info->ts);
            data.push_back(pac);
        }
        if (res == -1) {
            throw std::runtime_error(
                fmt::format("failed to read packets: {}", pcap_geterr(handle)));
        }
    } catch (const std::exception &e) {
        this->GetEventHandler()->CallAfter(std::bind(&MainFrame::notify_error, this, e.what()));
    }
}

void MainFrame::sniff_recv(std::vector<packet> data)
{
    typedef std::vector<packet>::iterator iter_t;
    pac_list.insert(pac_list.end(), std::move_iterator<iter_t>(data.begin()),
                    std::move_iterator<iter_t>(data.end()));
    m_list->SetItemCount(pac_list.size());
    m_list->ScrollList(0, m_list->GetViewRect().height - m_list->GetScrollPos(wxVERTICAL));
    m_list->Refresh();
}

void MainFrame::sniff_stopped()
{
    m_stop->Disable();
    m_start->Enable();
}

void MainFrame::show_json_prop(wxPGProperty *parent, const std::string &name, const json &j)
{
    if (j.is_array()) {
        wxPGProperty *p = nullptr;
        wxStringProperty *sp = new wxStringProperty(name, wxPG_LABEL, "<composed>");
        if (parent != nullptr) {
            p = m_prop->AppendIn(parent, sp);
        } else {
            p = m_prop->Append(sp);
        }
        int index = 0;
        for (auto it = j.begin(); it != j.end(); ++it) {
            show_json_prop(p, std::to_string(index), *it);
            ++index;
        }
    } else if (j.is_object()) {
        wxPGProperty *p = nullptr;
        wxStringProperty *sp = new wxStringProperty(name, wxPG_LABEL, "<composed>");
        if (parent != nullptr) {
            p = m_prop->AppendIn(parent, sp);
        } else {
            p = m_prop->Append(sp);
        }
        for (auto it = j.begin(); it != j.end(); ++it) {
            show_json_prop(p, it.key(), it.value());
        }
    } else if (j.is_string()) {
        auto p = new wxStringProperty(name, wxPG_LABEL, j.get<std::string>());
        if (parent != nullptr) {
            m_prop->AppendIn(parent, p);
        } else {
            m_prop->Append(p);
        }
    } else if (j.is_number()) {
        auto p = new wxIntProperty(name, wxPG_LABEL, j.get<long>());
        if (parent != nullptr) {
            m_prop->AppendIn(parent, p);
        } else {
            m_prop->Append(p);
        }
    } else if (j.is_boolean()) {
        auto p = new wxBoolProperty(name, wxPG_LABEL, j.get<bool>());
        if (parent != nullptr) {
            m_prop->AppendIn(parent, p);
        } else {
            m_prop->Append(p);
        }
    } else {
        auto p = new wxStringProperty(name, wxPG_LABEL, j.dump());
        if (parent != nullptr) {
            m_prop->AppendIn(parent, p);
        } else {
            m_prop->Append(p);
        }
    }
}

void MainFrame::notify_error(const std::string &msg)
{
    wxMessageDialog diag(this, msg, "Error", wxOK | wxICON_ERROR);
    diag.ShowModal();
}
