#include "prop-frame.h"

void PropFrame::setup_ui()
{
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
}
