#include "property-frame.h"
#include "packet-propgrid.h"

PropertyFrame::PropertyFrame(wxWindow *parent) : PropertyFrame_g(parent)
{
    m_prop->SetSplitterPosition(180);
    Bind(wxEVT_CLOSE_WINDOW, &PropertyFrame::on_close, this);
}

void PropertyFrame::show_packet(const packet &pac) { m_prop->show_packet(pac); }

void PropertyFrame::clear() { m_prop->Clear(); }

void PropertyFrame::on_close(wxCloseEvent &event) { Show(false); }
