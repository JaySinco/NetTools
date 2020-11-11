#pragma once
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#include <wx/grid.h>
#include "wx/aboutdlg.h"
#include <wx/propgrid/propgrid.h>
#endif
#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <gflags/gflags.h>
#include <glog/logging.h>

enum
{
    ID_QUIT = wxID_HIGHEST + 1,
    ID_ABOUT,
    ID_SNIFFSTART,
    ID_SNIFFSTOP,
};
