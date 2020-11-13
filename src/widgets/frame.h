#pragma once
#include "type.h"

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
    wxGrid *m_grid;
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

    void set_ui();
};
