#pragma once
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#include <wx/grid.h>
#endif
#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <gflags/gflags.h>
#include <glog/logging.h>

enum
{
    ntID_MENU_HELLO = wxID_HIGHEST + 1,
};
