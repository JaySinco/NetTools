///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Oct 26 2018)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
class PacketProp;
class SniffList;

#include <wx/string.h>
#include <wx/choice.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/textctrl.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/button.h>
#include <wx/listctrl.h>
#include <wx/menu.h>
#include <wx/statusbr.h>
#include <wx/frame.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>

///////////////////////////////////////////////////////////////////////////

#define ID_ADAPTORCHOICE 1000
#define ID_FILTERINPUT 1001
#define ID_SNIFFSTART 1002
#define ID_SNIFFSTOP 1003
#define ID_SNIFFCLEAR 1004
#define ID_SNIFFLIST 1005
#define ID_QUIT 1006
#define ID_ABOUT 1007
#define ID_STATUSBAR 1008
#define ID_SNIFFPROPGRID 1009

///////////////////////////////////////////////////////////////////////////////
/// Class MainFrameImpl
///////////////////////////////////////////////////////////////////////////////
class MainFrameImpl : public wxFrame
{
private:
protected:
    wxChoice *m_adaptor;
    wxTextCtrl *m_filter;
    wxButton *m_start;
    wxButton *m_stop;
    wxButton *m_clear;
    SniffList *m_list;
    wxMenuBar *m_menu;
    wxMenu *m_tools;
    wxMenu *m_help;
    wxStatusBar *m_status;

public:
    MainFrameImpl(wxWindow *parent, wxWindowID id = wxID_ANY,
                  const wxString &title = wxT("NetTools"), const wxPoint &pos = wxDefaultPosition,
                  const wxSize &size = wxSize(860, 620),
                  long style = wxDEFAULT_FRAME_STYLE | wxTAB_TRAVERSAL);

    ~MainFrameImpl();
};

///////////////////////////////////////////////////////////////////////////////
/// Class PropFrameImpl
///////////////////////////////////////////////////////////////////////////////
class PropFrameImpl : public wxFrame
{
private:
protected:
    PacketProp *m_prop;

public:
    PropFrameImpl(wxWindow *parent, wxWindowID id = wxID_ANY,
                  const wxString &title = wxT("Packet Property"),
                  const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxSize(400, 600),
                  long style = wxDEFAULT_FRAME_STYLE | wxTAB_TRAVERSAL);

    ~PropFrameImpl();
};
