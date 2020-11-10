#include "frame.h"

MainFrame::MainFrame(const wxPoint &pos, const wxSize &size)
    : wxFrame(NULL, wxID_ANY, "Hello World", pos, size)
{
    set_ui();

    Bind(wxEVT_MENU, &MainFrame::on_hello, this, ntID_MENU_HELLO);
    Bind(wxEVT_MENU, &MainFrame::on_about, this, wxID_ABOUT);
    Bind(wxEVT_MENU, &MainFrame::on_exit, this, wxID_EXIT);
}

void MainFrame::on_exit(wxCommandEvent &event) { Close(true); }

void MainFrame::on_about(wxCommandEvent &event)
{
    wxMessageBox("This is a wxWidgets Hello World example", "About Hello World",
                 wxOK | wxICON_INFORMATION);
}

void MainFrame::on_hello(wxCommandEvent &event) { LOG(INFO) << "Hello world from wxWidgets!"; }

void MainFrame::set_ui()
{
    wxBoxSizer *bSizer1;
    bSizer1 = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer *bSizer2;
    bSizer2 = new wxBoxSizer(wxHORIZONTAL);

    m_staticText1 =
        new wxStaticText(this, wxID_ANY, wxT("Adaptor"), wxDefaultPosition, wxDefaultSize, 0);
    m_staticText1->Wrap(-1);
    bSizer2->Add(m_staticText1, 0, wxALL, 5);

    wxArrayString m_choice1Choices;
    m_choice1 = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_choice1Choices, 0);
    m_choice1->SetSelection(0);
    bSizer2->Add(m_choice1, 0, wxALL, 5);

    bSizer1->Add(bSizer2, 1, wxEXPAND, 5);

    wxBoxSizer *bSizer3;
    bSizer3 = new wxBoxSizer(wxHORIZONTAL);

    wxBoxSizer *bSizer4;
    bSizer4 = new wxBoxSizer(wxVERTICAL);

    m_grid2 = new wxGrid(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);

    // Grid
    m_grid2->CreateGrid(5, 5);
    m_grid2->EnableEditing(true);
    m_grid2->EnableGridLines(true);
    m_grid2->EnableDragGridSize(false);
    m_grid2->SetMargins(0, 0);

    // Columns
    m_grid2->EnableDragColMove(false);
    m_grid2->EnableDragColSize(true);
    m_grid2->SetColLabelSize(30);
    m_grid2->SetColLabelAlignment(wxALIGN_CENTER, wxALIGN_CENTER);

    // Rows
    m_grid2->EnableDragRowSize(true);
    m_grid2->SetRowLabelSize(80);
    m_grid2->SetRowLabelAlignment(wxALIGN_CENTER, wxALIGN_CENTER);

    // Label Appearance

    // Cell Defaults
    m_grid2->SetDefaultCellAlignment(wxALIGN_LEFT, wxALIGN_TOP);
    bSizer4->Add(m_grid2, 1, wxALL | wxEXPAND, 5);

    bSizer3->Add(bSizer4, 1, wxEXPAND, 5);

    bSizer1->Add(bSizer3, 4, wxEXPAND, 5);

    this->SetSizer(bSizer1);
    this->Layout();
    m_statusBar1 = this->CreateStatusBar(1, wxSTB_SIZEGRIP, wxID_ANY);
    m_menubar1 = new wxMenuBar(0);
    m_menu1 = new wxMenu();
    wxMenuItem *m_menuItem1;
    m_menuItem1 = new wxMenuItem(
        m_menu1, ntID_MENU_HELLO, wxString(wxT("&Hello...")) + wxT('\t') + wxT("Ctrl-H"),
        wxT("Help string shown in status bar for this menu item"), wxITEM_NORMAL);
    m_menu1->Append(m_menuItem1);

    wxMenuItem *m_menuItem2;
    m_menuItem2 =
        new wxMenuItem(m_menu1, wxID_EXIT, wxString(wxEmptyString), wxEmptyString, wxITEM_NORMAL);
    m_menu1->Append(m_menuItem2);

    m_menubar1->Append(m_menu1, wxT("&File"));

    m_menu2 = new wxMenu();
    wxMenuItem *m_menuItem3;
    m_menuItem3 =
        new wxMenuItem(m_menu2, wxID_ABOUT, wxString(wxEmptyString), wxEmptyString, wxITEM_NORMAL);
    m_menu2->Append(m_menuItem3);

    m_menubar1->Append(m_menu2, wxT("&Help"));

    this->SetMenuBar(m_menubar1);

    this->Centre(wxBOTH);
}
