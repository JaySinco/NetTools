#include "main-frame.h"
#include "packet-listctrl.h"
#include "packet-propgrid.h"
#include "net/transport.h"
#include <wx/aboutdlg.h>
#include <thread>
#define NOTIFY_TRY try {
#define NOTIFY_CATCH                                                                  \
    }                                                                                 \
    catch (const std::runtime_error &e)                                               \
    {                                                                                 \
        std::string msg = e.what();                                                   \
        GetEventHandler()->CallAfter(std::bind(&MainFrame::notify_error, this, msg)); \
    }

MainFrame::MainFrame() : MainFrame_g(nullptr)
{
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
    m_list->init(&pac_list);
    m_prop->SetSplitterPosition(140);
    int status_width[] = {-12, -1};
    m_status->SetFieldsCount(2, status_width);
    update_status_total(0);
    column_sort.resize(PacketListCtrl::__FIELD_SIZE__, false);
    m_filter->SetFocus();

    Bind(wxEVT_MENU, &MainFrame::on_quit, this, ID_QUIT);
    Bind(wxEVT_MENU, &MainFrame::on_about, this, ID_ABOUT);
    m_start->Bind(wxEVT_BUTTON, &MainFrame::on_sniff_start, this);
    m_stop->Bind(wxEVT_BUTTON, &MainFrame::on_sniff_stop, this);
    m_clear->Bind(wxEVT_BUTTON, &MainFrame::on_sniff_clear, this);
    m_list->Bind(wxEVT_LIST_ITEM_SELECTED, &MainFrame::on_packet_selected, this);
    m_list->Bind(wxEVT_LIST_COL_CLICK, &MainFrame::on_list_col_clicked, this);
    m_filter->Bind(wxEVT_KILL_FOCUS, &MainFrame::on_filter_changed, this);
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
    std::thread(&MainFrame::sniff_background, this, apt, "", 500).detach();
}

void MainFrame::on_sniff_stop(wxCommandEvent &event) { sniff_should_stop = true; }

void MainFrame::on_sniff_clear(wxCommandEvent &event)
{
    m_list->DeleteAllItems();
    m_list->clear();
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
        auto sa = PacketListCtrl::stringfy_field(a, event.m_col);
        auto sb = PacketListCtrl::stringfy_field(b, event.m_col);
        return reverse ? sb < sa : sa < sb;
    });
    m_list->Refresh();
}

void MainFrame::on_filter_changed(wxFocusEvent &event)
{
    NOTIFY_TRY
    event.Skip();
    std::string filter = m_filter->GetValue();
    if (filter.size() == 0) {
        validator_.reset();
        return;
    }
    validator_ = validator::from_str(filter);
    NOTIFY_CATCH
}

void MainFrame::sniff_background(const adaptor &apt, const std::string &filter, int update_freq_ms)
{
    NOTIFY_TRY
    pcap_t *handle = transport::open_adaptor(apt, update_freq_ms);
    std::shared_ptr<void> handle_guard(nullptr, [&](void *) {
        pcap_close(handle);
        GetEventHandler()->CallAfter(std::bind(&MainFrame::sniff_stopped, this));
        VLOG(1) << "sniff stopped";
    });
    if (filter.size() > 0) {
        transport::setfilter(handle, filter, apt.mask);
    }
    VLOG(1) << "begin to sniff...";
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
        throw std::runtime_error("failed to read packets: {}"_format(pcap_geterr(handle)));
    }
    NOTIFY_CATCH
}

void MainFrame::sniff_recv(std::vector<packet> data)
{
    auto filter_end = std::remove_if(data.begin(), data.end(), [&](const packet &pac) {
        return validator_ && !validator_->test(pac);
    });

    using iter_t = std::vector<packet>::iterator;
    pac_list.insert(pac_list.end(), std::move_iterator<iter_t>(data.begin()),
                    std::move_iterator<iter_t>(filter_end));

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
    m_status->SetStatusText("Total:{:8d}"_format(n), 1);
}
