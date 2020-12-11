#pragma once
#include "prec.h"
#include "ui.h"
#include "net/packet.h"

class PropertyFrame : public PropertyFrame_g
{
public:
    PropertyFrame(wxWindow *parent);

    void show_packet(const packet &pac);
    void clear();

private:
    void on_close(wxCloseEvent &event);
};
