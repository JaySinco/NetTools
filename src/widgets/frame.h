#pragma once
#include "global.h"

class MainFrame : public wxFrame
{
public:
    MainFrame(const wxPoint &pos, const wxSize &size);

protected:
    wxStaticText *m_staticText1;
    wxChoice *m_choice1;
    wxGrid *m_grid2;
    wxStatusBar *m_statusBar1;
    wxMenuBar *m_menubar1;
    wxMenu *m_menu1;
    wxMenu *m_menu2;

private:
    void on_hello(wxCommandEvent &event);
    void on_exit(wxCommandEvent &event);
    void on_about(wxCommandEvent &event);

    void set_ui();
};
