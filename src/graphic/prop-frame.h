#pragma once
#include "prec.h"
#include "ui.h"
#include "net/transport.h"
#include "packet-prop.h"

class PropFrame : public PropFrameImpl
{
public:
    PropFrame(wxWindow *parent, const wxPoint &pos = wxDefaultPosition,
              const wxSize &size = wxDefaultSize);

    void show_packet(const packet &pac);
    void clear();

private:
    void on_close(wxCloseEvent &event);
};
