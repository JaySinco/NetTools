#pragma once
#include "prec.h"
#include "ui.h"
#include "net/transport.h"
#include "net/validator.h"
#include "sniff-list.h"
#include "prop-frame.h"
#include <atomic>

class MainFrame : public MainFrameImpl
{
public:
    MainFrame();

protected:
    PropFrame *m_prop_win;

private:
    void on_quit(wxCommandEvent &event);
    void on_about(wxCommandEvent &event);
    void on_sniff_start(wxCommandEvent &event);
    void on_sniff_stop(wxCommandEvent &event);
    void on_sniff_clear(wxCommandEvent &event);
    void on_packet_selected(wxListEvent &event);
    void on_list_col_clicked(wxListEvent &event);
    void on_filter_changed(wxFocusEvent &event);
    void sniff_background(const adaptor &apt, const std::string &filter, int update_freq_ms);
    void sniff_recv(std::vector<packet> data);
    void sniff_stopped();
    void notify_error(const std::string &msg);
    void update_status_total(size_t n);

    p_validator validator_;
    std::vector<packet> pac_list;
    std::vector<bool> column_sort;
    std::atomic<bool> sniff_should_stop;
};
