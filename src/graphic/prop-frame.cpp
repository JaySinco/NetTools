#include "prop-frame.h"

PropFrame::PropFrame(wxWindow *parent, const wxPoint &pos, const wxSize &size)
    : PropFrameImpl(parent)
{
    m_prop->SetSplitterPosition(180);
    Bind(wxEVT_CLOSE_WINDOW, &PropFrame::on_close, this);
}

void PropFrame::show_packet(const packet &pac) { m_prop->show_packet(pac); }

void PropFrame::clear() { m_prop->Clear(); }

void PropFrame::on_close(wxCloseEvent &event) { Show(false); }
