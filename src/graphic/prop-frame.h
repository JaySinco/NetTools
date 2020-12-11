#pragma once
#include "type.h"
#include "net/transport.h"
#include "packet-prop.h"

class PropFrame : public wxFrame
{
public:
    PropFrame(wxWindow *parent, const wxPoint &pos = wxDefaultPosition,
              const wxSize &size = wxDefaultSize);

    PacketProp *m_prop;

private:
    void on_close(wxCloseEvent &event);
    void setup_ui();
};
