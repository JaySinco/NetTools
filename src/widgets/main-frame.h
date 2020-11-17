#pragma once
#include "type.h"
#include "core/transport.h"
#include <atomic>

class MainFrame : public wxFrame
{
public:
    MainFrame(const wxPoint &pos, const wxSize &size);

protected:
    wxChoice *m_adaptor;
    wxTextCtrl *m_filter;
    wxButton *m_start;
    wxButton *m_stop;
    wxButton *m_clear;
    wxListCtrl *m_list;
    wxPropertyGrid *m_prop;
    wxMenuBar *m_menu;
    wxMenu *m_tools;
    wxMenu *m_help;

private:
    void on_quit(wxCommandEvent &event);
    void on_about(wxCommandEvent &event);
    void on_sniff_start(wxCommandEvent &event);
    void on_sniff_stop(wxCommandEvent &event);
    void on_sniff_clear(wxCommandEvent &event);
    void on_packet_selected(wxListEvent &event);
    void sniff_background(const adaptor &apt, const std::string &filter, int update_freq_ms);
    void sniff_recv(std::vector<packet> data);
    void sniff_stopped();
    void show_json_prop(wxPGProperty *parent, const std::string &name, const json &j);
    void notify_error(const std::string &msg);
    void setup_ui();

    std::vector<packet> pac_list;
    std::atomic<bool> sniff_should_stop;
};
