#include "main-frame.h"
#include "packet-listctrl.h"
#include "packet-propgrid.h"
#include "net/transport.h"
#include <wx/msgdlg.h>
#include <execution>
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
    m_list->init(&idx_list, &pac_list);
    m_prop->SetSplitterPosition(140);
    int status_width[] = {97, -1};
    m_status->SetFieldsCount(2, status_width);
    update_status_bar();
    column_sort.resize(PacketListCtrl::__FIELD_SIZE__, false);
    m_filter->SetFocus();
    std::thread(&MainFrame::port_table_update_background, this, 10).detach();
    m_start->Bind(wxEVT_BUTTON, &MainFrame::on_sniff_start, this);
    m_stop->Bind(wxEVT_BUTTON, &MainFrame::on_sniff_stop, this);
    m_clear->Bind(wxEVT_BUTTON, &MainFrame::on_sniff_clear, this);
    m_list->Bind(wxEVT_LIST_ITEM_SELECTED, &MainFrame::on_packet_selected, this);
    m_list->Bind(wxEVT_LIST_COL_CLICK, &MainFrame::on_list_col_clicked, this);
    m_filter->Bind(wxEVT_TEXT_ENTER, &MainFrame::on_filter_changed, this);
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
    m_list->clear();
    m_prop->Clear();
    idx_list.clear();
    idx_list.shrink_to_fit();
    pac_list.clear();
    pac_list.shrink_to_fit();
    port_table::clear();
    update_status_bar();
}

void MainFrame::on_packet_selected(wxListEvent &event)
{
    m_prop->show_packet(pac_list.at(idx_list.at(event.m_itemIndex)));
}

void MainFrame::on_list_col_clicked(wxListEvent &event)
{
    bool reverse = column_sort.at(event.m_col);
    column_sort.at(event.m_col) = !reverse;
    std::stable_sort(std::execution::par_unseq, idx_list.begin(), idx_list.end(),
                     [&](size_t a, size_t b) {
                         auto sa = PacketListCtrl::stringfy_field(pac_list.at(a), event.m_col);
                         auto sb = PacketListCtrl::stringfy_field(pac_list.at(b), event.m_col);
                         return reverse ? sb < sa : sa < sb;
                     });
    m_list->Refresh();
}

void MainFrame::on_filter_changed(wxCommandEvent &event)
{
    NOTIFY_TRY
    std::string filter = m_filter->GetValue();
    if (filter.size() == 0) {
        filter_.reset();
    } else {
        filter_ = filter::from_str(filter);
    }
    idx_list.clear();
    for (size_t i = 0; i < pac_list.size(); ++i) {
        if (!filter_ || filter_->test(pac_list[i])) {
            idx_list.push_back(i);
        }
    }
    m_list->SetItemCount(idx_list.size());
    m_list->Refresh();
    update_status_bar();
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
        pac.to_json();
        data.push_back(pac);
    }
    if (res == -1) {
        throw std::runtime_error("failed to read packets: {}"_format(pcap_geterr(handle)));
    }
    NOTIFY_CATCH
}

void MainFrame::port_table_update_background(int update_freq_ms)
{
    while (true) {
        port_table::update();
        std::this_thread::sleep_for(std::chrono::milliseconds(update_freq_ms));
    }
}

void MainFrame::sniff_recv(std::vector<packet> data)
{
    bool need_refresh = false;
    for (auto &pac : data) {
        pac_list.push_back(std::move(pac));
        if (!filter_ || filter_->test(pac_list.back())) {
            need_refresh = true;
            idx_list.push_back(pac_list.size() - 1);
        }
    }
    if (need_refresh) {
        m_list->SetItemCount(idx_list.size());
        m_list->Refresh();
    }
    update_status_bar();
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

void MainFrame::update_status_bar()
{
    size_t filtered = idx_list.size();
    size_t total = pac_list.size();
    if (filter_) {
        m_status->SetStatusText(" Filtered: {}"_format(filtered), 1);
    } else {
        m_status->SetStatusText(" No Filter Set", 1);
    }
    m_status->SetStatusText(" Total: {}"_format(total), 0);
}
