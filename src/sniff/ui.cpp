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

    m_splitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                      wxSP_3D | wxSP_THIN_SASH);
    m_splitter->SetSashGravity(0.625);
    m_splitter->Connect(wxEVT_IDLE, wxIdleEventHandler(MainFrame_g::m_splitterOnIdle), NULL, this);

    m_panel_left =
        new wxPanel(m_splitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    wxBoxSizer *bSizer4;
    bSizer4 = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer *bSizer6;
    bSizer6 = new wxBoxSizer(wxHORIZONTAL);

    wxArrayString m_adaptorChoices;
    m_adaptor =
        new wxChoice(m_panel_left, wxID_ANY, wxDefaultPosition, wxDefaultSize, m_adaptorChoices, 0);
    m_adaptor->SetSelection(0);
    bSizer6->Add(m_adaptor, 1, wxBOTTOM | wxEXPAND | wxFIXED_MINSIZE | wxRIGHT | wxTOP, 3);

    m_filter = new wxTextCtrl(m_panel_left, wxID_ANY, wxEmptyString, wxDefaultPosition,
                              wxDefaultSize, wxTE_PROCESS_ENTER);
    bSizer6->Add(m_filter, 4, wxBOTTOM | wxEXPAND | wxLEFT | wxTOP, 3);

    bSizer4->Add(bSizer6, 1, wxEXPAND, 5);

    m_list = new PacketListCtrl(m_panel_left, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                wxLC_ICON | wxLC_REPORT | wxLC_VIRTUAL);
    m_list->SetFont(wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false,
                           wxT("Consolas")));

    bSizer4->Add(m_list, 24, wxEXPAND, 3);

    m_panel_left->SetSizer(bSizer4);
    m_panel_left->Layout();
    bSizer4->Fit(m_panel_left);
    m_panel_right =
        new wxPanel(m_splitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    wxBoxSizer *bSizer10;
    bSizer10 = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer *bSizer8;
    bSizer8 = new wxBoxSizer(wxHORIZONTAL);

    m_start =
        new wxButton(m_panel_right, wxID_ANY, wxT("Start"), wxDefaultPosition, wxDefaultSize, 0);
    bSizer8->Add(m_start, 1, wxBOTTOM | wxEXPAND | wxRIGHT | wxTOP, 3);

    m_stop =
        new wxButton(m_panel_right, wxID_ANY, wxT("Stop"), wxDefaultPosition, wxDefaultSize, 0);
    bSizer8->Add(m_stop, 1, wxALL | wxEXPAND, 3);

    m_clear =
        new wxButton(m_panel_right, wxID_ANY, wxT("Clear"), wxDefaultPosition, wxDefaultSize, 0);
    bSizer8->Add(m_clear, 1, wxBOTTOM | wxEXPAND | wxLEFT | wxTOP, 3);

    bSizer10->Add(bSizer8, 1, wxEXPAND, 5);

    m_prop = new PacketPropGrid(m_panel_right, wxID_ANY, wxDefaultPosition, wxSize(-1, -1),
                                wxPG_DEFAULT_STYLE | wxHSCROLL | wxVSCROLL);
    bSizer10->Add(m_prop, 24, wxEXPAND, 3);

    m_panel_right->SetSizer(bSizer10);
    m_panel_right->Layout();
    bSizer10->Fit(m_panel_right);
    m_splitter->SplitVertically(m_panel_left, m_panel_right, 675);
    bSizer1->Add(m_splitter, 1, wxEXPAND, 2);

    this->SetSizer(bSizer1);
    this->Layout();
    m_status = this->CreateStatusBar(1, wxSTB_SIZEGRIP, wxID_ANY);

    this->Centre(wxBOTH);
}

MainFrame_g::~MainFrame_g() {}
