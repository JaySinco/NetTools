#pragma once
#include "global.h"

class MainFrame : public wxFrame
{
public:
    MainFrame(const wxPoint &pos, const wxSize &size);

protected:
    wxChoice *m_choice1;
    wxTextCtrl *m_textCtrl1;
    wxButton *m_button2;
    wxButton *m_button1;
    wxGrid *m_grid2;
    wxPropertyGrid *m_propertyGrid1;
    wxStatusBar *m_statusBar1;
    wxMenuBar *m_menubar1;
    wxMenu *m_menu1;
    wxMenu *m_menu2;

private:
    void on_quit(wxCommandEvent &event);
    void on_about(wxCommandEvent &event);
    void on_sniff_start(wxCommandEvent &event);
    void on_sniff_stop(wxCommandEvent &event);

    void set_ui();
};
