#include "frame.h"

MainFrame::MainFrame(const wxPoint &pos, const wxSize &size)
    : wxFrame(NULL, wxID_ANY, "NetTools", pos, size)
{
    set_ui();

    Bind(wxEVT_MENU, &MainFrame::on_quit, this, ID_QUIT);
    Bind(wxEVT_MENU, &MainFrame::on_about, this, ID_ABOUT);
    Bind(wxEVT_BUTTON, &MainFrame::on_sniff_start, this, ID_SNIFFSTART);
    Bind(wxEVT_BUTTON, &MainFrame::on_sniff_stop, this, ID_SNIFFSTOP);
}

void MainFrame::on_quit(wxCommandEvent &event) { Close(true); }

void MainFrame::on_about(wxCommandEvent &event)
{
    wxAboutDialogInfo info;
    info.SetName("NetTools");
    info.SetDescription("Network Toolset Program");
    info.AddDeveloper("Jaysinco");
    info.SetWebSite("https://github.com/JaySinco/NetTools");
    wxAboutBox(info, this);
}

void MainFrame::on_sniff_start(wxCommandEvent &event) { LOG(INFO) << "sniff start!"; }

void MainFrame::on_sniff_stop(wxCommandEvent &event) { LOG(INFO) << "sniff end!"; }

void MainFrame::set_ui()
{
    wxBoxSizer *bSizer1;
    bSizer1 = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer *bSizer2;
    bSizer2 = new wxBoxSizer(wxHORIZONTAL);

    wxBoxSizer *bSizer6;
    bSizer6 = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer *bSizer7;
    bSizer7 = new wxBoxSizer(wxHORIZONTAL);

    wxArrayString m_choice1Choices;
    m_choice1 = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_choice1Choices, 0);
    m_choice1->SetSelection(0);
    bSizer7->Add(m_choice1, 2, wxALL | wxEXPAND, 5);

    bSizer7->Add(0, 0, 3, wxEXPAND, 5);

    bSizer6->Add(bSizer7, 1, wxEXPAND, 5);

    wxBoxSizer *bSizer8;
    bSizer8 = new wxBoxSizer(wxHORIZONTAL);

    m_textCtrl1 =
        new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
    bSizer8->Add(m_textCtrl1, 3, wxALL | wxEXPAND, 5);

    m_button2 =
        new wxButton(this, ID_SNIFFSTART, wxT("Start"), wxDefaultPosition, wxDefaultSize, 0);
    bSizer8->Add(m_button2, 0, wxALL, 5);

    m_button1 = new wxButton(this, ID_SNIFFSTOP, wxT("Stop"), wxDefaultPosition, wxDefaultSize, 0);
    bSizer8->Add(m_button1, 0, wxALL, 5);

    bSizer6->Add(bSizer8, 1, wxEXPAND, 5);

    bSizer2->Add(bSizer6, 1, wxEXPAND, 5);

    bSizer1->Add(bSizer2, 1, wxEXPAND, 5);

    wxBoxSizer *bSizer3;
    bSizer3 = new wxBoxSizer(wxHORIZONTAL);

    wxBoxSizer *bSizer4;
    bSizer4 = new wxBoxSizer(wxHORIZONTAL);

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

    bSizer3->Add(bSizer4, 5, wxEXPAND, 5);

    wxBoxSizer *bSizer5;
    bSizer5 = new wxBoxSizer(wxHORIZONTAL);

    m_propertyGrid1 =
        new wxPropertyGrid(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxPG_DEFAULT_STYLE);
    bSizer5->Add(m_propertyGrid1, 1, wxALL | wxEXPAND, 5);

    bSizer3->Add(bSizer5, 3, wxEXPAND, 5);

    bSizer1->Add(bSizer3, 8, wxEXPAND, 5);

    this->SetSizer(bSizer1);
    this->Layout();
    m_statusBar1 = this->CreateStatusBar(1, wxSTB_SIZEGRIP, wxID_ANY);
    m_menubar1 = new wxMenuBar(0);
    m_menu1 = new wxMenu();
    wxMenuItem *m_menuItem2;
    m_menuItem2 =
        new wxMenuItem(m_menu1, ID_QUIT, wxString(wxT("Quit")), wxEmptyString, wxITEM_NORMAL);
    m_menu1->Append(m_menuItem2);

    m_menubar1->Append(m_menu1, wxT("&Tools"));

    m_menu2 = new wxMenu();
    wxMenuItem *m_menuItem3;
    m_menuItem3 =
        new wxMenuItem(m_menu2, ID_ABOUT, wxString(wxT("About")), wxEmptyString, wxITEM_NORMAL);
    m_menu2->Append(m_menuItem3);

    m_menubar1->Append(m_menu2, wxT("&Help"));

    this->SetMenuBar(m_menubar1);

    this->Centre(wxBOTH);
}
