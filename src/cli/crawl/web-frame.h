#pragma once
#include "prec.h"
#include "ui.h"

class WebFrame : public WebFrame_g
{
public:
    WebFrame();

private:
    void OnUrl(wxCommandEvent &event);
};
