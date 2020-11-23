#include "main-frame.h"
#include <thread>

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
    m_list->SetDataPtr(&pac_list);
    m_prop->SetSplitterPosition(180);
    int status_width[] = {-9, -1};
    m_status->SetFieldsCount(2, status_width);
    update_status_total(0);
    column_sort.resize(SniffList::__FIELD_SIZE__, false);

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
    update_status_total(0);
}

void MainFrame::on_packet_selected(wxListEvent &event)
{
    m_prop->show_packet(pac_list.at(event.m_itemIndex));
}

void MainFrame::on_list_col_clicked(wxListEvent &event)
{
    if (!m_start->IsEnabled()) {
        return;
    }
    bool reverse = column_sort.at(event.m_col);
    column_sort.at(event.m_col) = !reverse;
    std::stable_sort(pac_list.begin(), pac_list.end(), [&](const packet &a, const packet &b) {
        auto sa = SniffList::stringfy_field(a, event.m_col);
        auto sb = SniffList::stringfy_field(b, event.m_col);
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
            GetEventHandler()->CallAfter(std::bind(&MainFrame::sniff_stopped, this));
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
                    GetEventHandler()->CallAfter(
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
        GetEventHandler()->CallAfter(std::bind(&MainFrame::notify_error, this, e.what()));
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
    update_status_total(pac_list.size());
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

void MainFrame::update_status_total(size_t n)
{
    m_status->SetStatusText(fmt::format("Total:{:8d}", n), 1);
}
