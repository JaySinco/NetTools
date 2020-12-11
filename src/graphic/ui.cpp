///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Oct 26 2018)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "packet-prop.h"
#include "sniff-list.h"

#include "ui.h"

///////////////////////////////////////////////////////////////////////////

MainFrameImpl::MainFrameImpl(wxWindow *parent, wxWindowID id, const wxString &title,
                             const wxPoint &pos, const wxSize &size, long style)
    : wxFrame(parent, id, title, pos, size, style)
{
    this->SetSizeHints(wxDefaultSize, wxDefaultSize);
    this->SetFont(wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false,
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
    bSizer7->Add(m_adaptor, 1, wxEXPAND | wxFIXED_MINSIZE | wxLEFT | wxRIGHT | wxTOP, 3);

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
    bSizer3 = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer *bSizer4;
    bSizer4 = new wxBoxSizer(wxHORIZONTAL);

    m_list = new SniffList(this, ID_SNIFFLIST, wxDefaultPosition, wxDefaultSize,
                           wxLC_ICON | wxLC_REPORT | wxLC_VIRTUAL);
    m_list->SetFont(wxFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false,
                           wxT("Consolas")));

    bSizer4->Add(m_list, 1, wxBOTTOM | wxEXPAND | wxLEFT | wxRIGHT, 3);

    bSizer3->Add(bSizer4, 5, wxEXPAND, 5);

    wxBoxSizer *bSizer5;
    bSizer5 = new wxBoxSizer(wxHORIZONTAL);

    bSizer3->Add(bSizer5, 5, wxEXPAND, 5);

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

    m_status = this->CreateStatusBar(1, wxSTB_SIZEGRIP, ID_STATUSBAR);

    this->Centre(wxBOTH);
}

MainFrameImpl::~MainFrameImpl() {}

PropFrameImpl::PropFrameImpl(wxWindow *parent, wxWindowID id, const wxString &title,
                             const wxPoint &pos, const wxSize &size, long style)
    : wxFrame(parent, id, title, pos, size, style)
{
    this->SetSizeHints(wxDefaultSize, wxDefaultSize);
    this->SetFont(wxFont(wxNORMAL_FONT->GetPointSize(), wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL,
                         wxFONTWEIGHT_NORMAL, false, wxT("Arial")));

    wxBoxSizer *bSizer9;
    bSizer9 = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer *bSizer10;
    bSizer10 = new wxBoxSizer(wxHORIZONTAL);

    m_prop = new PacketProp(this, ID_SNIFFPROPGRID, wxDefaultPosition, wxSize(-1, -1),
                            wxPG_DEFAULT_STYLE);
    bSizer10->Add(m_prop, 3, wxEXPAND, 3);

    bSizer9->Add(bSizer10, 1, wxEXPAND, 5);

    this->SetSizer(bSizer9);
    this->Layout();

    this->Centre(wxBOTH);
}

PropFrameImpl::~PropFrameImpl() {}
