///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Oct 26 2018)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "packet-listctrl.h"
#include "packet-propgrid.h"

#include "ui.h"

///////////////////////////////////////////////////////////////////////////

MainFrame_g::MainFrame_g(wxWindow *parent, wxWindowID id, const wxString &title, const wxPoint &pos,
                         const wxSize &size, long style)
    : wxFrame(parent, id, title, pos, size, style)
{
    this->SetSizeHints(wxDefaultSize, wxDefaultSize);
    this->SetFont(wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false,
                         wxT("Arial")));

    wxBoxSizer *bSizer1;
    bSizer1 = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer *bSizer2;
    bSizer2 = new wxBoxSizer(wxHORIZONTAL);

    m_panel4 = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    m_panel4->SetFont(wxFont(11, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL,
                             false, wxT("Arial")));

    wxBoxSizer *bSizer6;
    bSizer6 = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer *bSizer7;
    bSizer7 = new wxBoxSizer(wxHORIZONTAL);

    wxArrayString m_adaptorChoices;
    m_adaptor =
        new wxChoice(m_panel4, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_adaptorChoices, 0);
    m_adaptor->SetSelection(0);
    bSizer7->Add(m_adaptor, 1, wxEXPAND | wxFIXED_MINSIZE | wxRIGHT | wxTOP, 3);

    bSizer7->Add(0, 0, 4, wxEXPAND, 5);

    bSizer6->Add(bSizer7, 1, wxEXPAND, 5);

    wxBoxSizer *bSizer8;
    bSizer8 = new wxBoxSizer(wxHORIZONTAL);

    m_filter =
        new wxTextCtrl(m_panel4, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
    bSizer8->Add(m_filter, 9, wxBOTTOM | wxEXPAND | wxRIGHT | wxTOP, 3);

    m_start = new wxButton(m_panel4, wxID_ANY, wxT("Start"), wxDefaultPosition, wxDefaultSize, 0);
    bSizer8->Add(m_start, 1, wxALL | wxEXPAND, 3);

    m_stop = new wxButton(m_panel4, wxID_ANY, wxT("Stop"), wxDefaultPosition, wxDefaultSize, 0);
    bSizer8->Add(m_stop, 1, wxALL | wxEXPAND, 3);

    m_clear = new wxButton(m_panel4, wxID_ANY, wxT("Clear"), wxDefaultPosition, wxDefaultSize, 0);
    bSizer8->Add(m_clear, 1, wxBOTTOM | wxEXPAND | wxLEFT | wxTOP, 3);

    bSizer6->Add(bSizer8, 1, wxEXPAND, 5);

    m_panel4->SetSizer(bSizer6);
    m_panel4->Layout();
    bSizer6->Fit(m_panel4);
    bSizer2->Add(m_panel4, 1, wxEXPAND, 2);

    bSizer1->Add(bSizer2, 1, wxEXPAND, 5);

    wxBoxSizer *bSizer3;
    bSizer3 = new wxBoxSizer(wxVERTICAL);

    m_splitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                      wxSP_3D | wxSP_THIN_SASH);
    m_splitter->SetSashGravity(0.625);
    m_splitter->Connect(wxEVT_IDLE, wxIdleEventHandler(MainFrame_g::m_splitterOnIdle), NULL, this);

    m_panel_left =
        new wxPanel(m_splitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    wxBoxSizer *bSizer4;
    bSizer4 = new wxBoxSizer(wxHORIZONTAL);

    m_list = new PacketListCtrl(m_panel_left, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                wxLC_ICON | wxLC_REPORT | wxLC_VIRTUAL);
    m_list->SetFont(wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false,
                           wxT("Consolas")));

    bSizer4->Add(m_list, 1, wxEXPAND, 3);

    m_panel_left->SetSizer(bSizer4);
    m_panel_left->Layout();
    bSizer4->Fit(m_panel_left);
    m_panel_right =
        new wxPanel(m_splitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    wxBoxSizer *bSizer10;
    bSizer10 = new wxBoxSizer(wxHORIZONTAL);

    m_prop = new PacketPropGrid(m_panel_right, wxID_ANY, wxDefaultPosition, wxSize(-1, -1),
                                wxPG_DEFAULT_STYLE | wxHSCROLL | wxVSCROLL);
    bSizer10->Add(m_prop, 3, wxEXPAND, 3);

    m_panel_right->SetSizer(bSizer10);
    m_panel_right->Layout();
    bSizer10->Fit(m_panel_right);
    m_splitter->SplitVertically(m_panel_left, m_panel_right, 922);
    bSizer3->Add(m_splitter, 1, wxEXPAND, 2);

    bSizer1->Add(bSizer3, 9, wxEXPAND, 5);

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

    m_status = this->CreateStatusBar(1, wxSTB_SIZEGRIP, wxID_ANY);

    this->Centre(wxBOTH);
}

MainFrame_g::~MainFrame_g() {}
