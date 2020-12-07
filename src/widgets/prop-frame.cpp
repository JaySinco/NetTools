#include "prop-frame.h"

PropFrame::PropFrame(wxWindow *parent, const wxPoint &pos, const wxSize &size)
    : wxFrame(parent, wxID_ANY, "Packet Property", pos, size)
{
    setup_ui();
    m_prop->SetSplitterPosition(180);
    Bind(wxEVT_CLOSE_WINDOW, &PropFrame::on_close, this);
}

void PropFrame::on_close(wxCloseEvent &event) { Show(false); }
