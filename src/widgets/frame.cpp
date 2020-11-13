#include "core/transport.h"
#include "frame.h"

MainFrame::MainFrame(const wxPoint &pos, const wxSize &size)
    : wxFrame(NULL, wxID_ANY, "NetTools", pos, size)
{
    set_ui();
    for (const auto &apt : adaptor::all()) {
        m_adaptor->AppendString(apt.ip.to_str());
    }
    m_adaptor->SetSelection(0);
    m_stop->Disable();

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
    info.AddDeveloper("Mr.Robot");
    info.SetWebSite("https://github.com/JaySinco/NetTools");
    wxAboutBox(info, this);
}

void MainFrame::on_sniff_start(wxCommandEvent &event)
{
    m_start->Disable();
    m_stop->Enable();
}

void MainFrame::on_sniff_stop(wxCommandEvent &event)
{
    m_stop->Disable();
    m_start->Enable();
}

void MainFrame::on_sniff_clear(wxCommandEvent &event) {}

void MainFrame::set_ui()
{
    this->SetFont(wxFont(11, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false,
                         wxT("Arial")));

    wxBoxSizer *bSizer1;
    bSizer1 = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer *bSizer2;
    bSizer2 = new wxBoxSizer(wxHORIZONTAL);

    wxBoxSizer *bSizer6;
    bSizer6 = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer *bSizer7;
    bSizer7 = new wxBoxSizer(wxHORIZONTAL);

    wxArrayString m_adaptorChoices;
    m_adaptor =
        new wxChoice(this, ID_ADAPTORCHOICE, wxDefaultPosition, wxDefaultSize, m_adaptorChoices, 0);
    m_adaptor->SetSelection(0);
    bSizer7->Add(m_adaptor, 1, wxALL | wxEXPAND | wxFIXED_MINSIZE, 3);

    bSizer7->Add(0, 0, 4, wxEXPAND, 5);

    bSizer6->Add(bSizer7, 1, wxEXPAND, 5);

    wxBoxSizer *bSizer8;
    bSizer8 = new wxBoxSizer(wxHORIZONTAL);

    m_filter =
        new wxTextCtrl(this, ID_FILTERINPUT, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
    bSizer8->Add(m_filter, 5, wxALL | wxEXPAND, 3);

    m_start = new wxButton(this, ID_SNIFFSTART, wxT("Start"), wxDefaultPosition, wxDefaultSize, 0);
    bSizer8->Add(m_start, 1, wxALL | wxEXPAND, 3);

    m_stop = new wxButton(this, ID_SNIFFSTOP, wxT("Stop"), wxDefaultPosition, wxDefaultSize, 0);
    bSizer8->Add(m_stop, 1, wxALL | wxEXPAND, 3);

    m_clear = new wxButton(this, ID_SNIFFCLEAR, wxT("Clear"), wxDefaultPosition, wxDefaultSize, 0);
    bSizer8->Add(m_clear, 1, wxALL | wxEXPAND, 3);

    bSizer6->Add(bSizer8, 1, wxEXPAND, 5);

    bSizer2->Add(bSizer6, 1, wxEXPAND, 5);

    bSizer1->Add(bSizer2, 1, wxEXPAND, 5);

    wxBoxSizer *bSizer3;
    bSizer3 = new wxBoxSizer(wxHORIZONTAL);

    wxBoxSizer *bSizer4;
    bSizer4 = new wxBoxSizer(wxHORIZONTAL);

    m_grid = new wxGrid(this, ID_SNIFFGRID, wxDefaultPosition, wxDefaultSize, 0);

    // Grid
    m_grid->CreateGrid(5, 5);
    m_grid->EnableEditing(true);
    m_grid->EnableGridLines(true);
    m_grid->EnableDragGridSize(false);
    m_grid->SetMargins(0, 0);

    // Columns
    m_grid->EnableDragColMove(false);
    m_grid->EnableDragColSize(true);
    m_grid->SetColLabelSize(30);
    m_grid->SetColLabelAlignment(wxALIGN_CENTER, wxALIGN_CENTER);

    // Rows
    m_grid->EnableDragRowSize(true);
    m_grid->SetRowLabelSize(80);
    m_grid->SetRowLabelAlignment(wxALIGN_CENTER, wxALIGN_CENTER);

    // Label Appearance

    // Cell Defaults
    m_grid->SetDefaultCellAlignment(wxALIGN_LEFT, wxALIGN_TOP);
    bSizer4->Add(m_grid, 5, wxALL | wxEXPAND, 3);

    bSizer3->Add(bSizer4, 5, wxEXPAND, 5);

    wxBoxSizer *bSizer5;
    bSizer5 = new wxBoxSizer(wxHORIZONTAL);

    m_prop = new wxPropertyGrid(this, ID_SNIFFPROPGRID, wxDefaultPosition, wxDefaultSize,
                                wxPG_DEFAULT_STYLE);
    bSizer5->Add(m_prop, 3, wxBOTTOM | wxEXPAND | wxRIGHT | wxTOP, 3);

    bSizer3->Add(bSizer5, 3, wxEXPAND, 5);

    bSizer1->Add(bSizer3, 8, wxEXPAND, 5);

    this->SetSizer(bSizer1);
    this->Layout();
    m_menu = new wxMenuBar(0);
    m_tools = new wxMenu();
    wxMenuItem *m_quit;
    m_quit = new wxMenuItem(m_tools, ID_QUIT, wxString(wxT("Quit")), wxEmptyString, wxITEM_NORMAL);
    m_tools->Append(m_quit);

    m_menu->Append(m_tools, wxT("&Tools"));

    m_help = new wxMenu();
    wxMenuItem *m_about;
    m_about =
        new wxMenuItem(m_help, ID_ABOUT, wxString(wxT("About")), wxEmptyString, wxITEM_NORMAL);
    m_help->Append(m_about);

    m_menu->Append(m_help, wxT("&Help"));

    this->SetMenuBar(m_menu);

    this->Centre(wxBOTH);
}
