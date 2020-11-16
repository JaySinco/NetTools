#include "main-frame.h"
#include "core/ethernet.h"
#include "core/ipv4.h"
#include "core/udp.h"
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
    for (const auto &apt : adaptor::all()) {
        m_adaptor->AppendString(apt.ip.to_str());
    }
    m_adaptor->SetSelection(0);
    m_stop->Disable();
    m_list->AppendColumn("time", wxLIST_FORMAT_LEFT, 100);
    m_list->AppendColumn("smac", wxLIST_FORMAT_LEFT, 135);
    m_list->AppendColumn("dmac", wxLIST_FORMAT_LEFT, 135);
    m_list->AppendColumn("sip", wxLIST_FORMAT_LEFT, 115);
    m_list->AppendColumn("dip", wxLIST_FORMAT_LEFT, 115);
    m_list->AppendColumn("sport", wxLIST_FORMAT_LEFT, 50);
    m_list->AppendColumn("dport", wxLIST_FORMAT_LEFT, 50);
    m_list->AppendColumn("type", wxLIST_FORMAT_LEFT, 50);

    Bind(wxEVT_MENU, &MainFrame::on_quit, this, ID_QUIT);
    Bind(wxEVT_MENU, &MainFrame::on_about, this, ID_ABOUT);
    Bind(wxEVT_BUTTON, &MainFrame::on_sniff_start, this, ID_SNIFFSTART);
    Bind(wxEVT_BUTTON, &MainFrame::on_sniff_stop, this, ID_SNIFFSTOP);
    Bind(wxEVT_BUTTON, &MainFrame::on_sniff_clear, this, ID_SNIFFCLEAR);
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
    pac_list.clear();
}

void MainFrame::sniff_background(const adaptor &apt, const std::string &filter, int update_freq_ms)
{
    try {
        pcap_t *handle = transport::open_adaptor(apt);
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
            if (res == 0) {
                continue;  // timeout elapsed
            }
            packet pac(start, start + info->len, info->ts);
            data.push_back(pac);
            auto now = std::chrono::system_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_tm);
            if (duration.count() >= update_freq_ms) {
                this->GetEventHandler()->CallAfter(
                    std::bind(&MainFrame::sniff_recv, this, std::move(data)));
                data.clear();
                start_tm = now;
            }
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
    for (packet &pac : data) {
        long idx = pac_list.size();
        wxListItem item;
        m_list->InsertItem(idx, item);
        m_list->SetItem(idx, FIELD_TIME, tv2s(pac.get_detail().time));
        const auto &layers = pac.get_detail().layers;
        if (layers.size() > 0) {
            if (layers.front()->type() == Protocol_Type_Ethernet) {
                const auto &eh = dynamic_cast<const ethernet &>(*layers.front());
                m_list->SetItem(idx, FIELD_SOURCE_MAC, eh.get_detail().smac.to_str());
                m_list->SetItem(idx, FIELD_DEST_MAC, eh.get_detail().dmac.to_str());
            }
            if (layers.size() > 1 && layers[1]->type() == Protocol_Type_IPv4) {
                const auto &ih = dynamic_cast<const ipv4 &>(*layers[1]);
                m_list->SetItem(idx, FIELD_SOURCE_IP, ih.get_detail().sip.to_str());
                m_list->SetItem(idx, FIELD_DEST_IP, ih.get_detail().dip.to_str());
            }
            if (layers.size() > 2 && layers[2]->type() == Protocol_Type_UDP) {
                const auto &uh = dynamic_cast<const udp &>(*layers[2]);
                m_list->SetItem(idx, FIELD_SOURCE_PORT, std::to_string(uh.get_detail().sport));
                m_list->SetItem(idx, FIELD_DEST_PORT, std::to_string(uh.get_detail().dport));
            }
            std::string type = layers.back()->succ_type();
            if (type == Protocol_Type_Void || type.find("unknow(") != std::string::npos) {
                type = layers.back()->type();
            }
            m_list->SetItem(idx, FIELD_TYPE, type);
        }
        pac_list.push_back(std::move(pac));
    }
    m_list->ScrollList(0, m_list->GetViewRect().height - m_list->GetScrollPos(wxVERTICAL));
}

void MainFrame::sniff_stopped()
{
    m_stop->Disable();
    m_start->Enable();
}

void MainFrame::notify_error(const std::string &msg)
{
    wxMessageDialog diag(this, msg, "Error", wxOK | wxICON_ERROR);
    diag.ShowModal();
}
