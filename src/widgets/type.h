#pragma once
#include "prec.h"
#include <wx/listctrl.h>
#include <wx/aboutdlg.h>
#include <wx/propgrid/propgrid.h>

enum
{
    ID_QUIT = wxID_HIGHEST + 1,
    ID_ABOUT,
    ID_SNIFFSTART,
    ID_SNIFFSTOP,
    ID_SNIFFCLEAR,
    ID_SNIFFLIST,
    ID_SNIFFPROPGRID,
    ID_ADAPTORCHOICE,
    ID_FILTERINPUT,
};
