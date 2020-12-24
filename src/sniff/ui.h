///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Oct 26 2018)
// http://www.wxformbuilder.org/
//
// PLEASE DO *NOT* EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/artprov.h>
#include <wx/xrc/xmlres.h>
class PacketListCtrl;
class PacketPropGrid;

#include <wx/string.h>
#include <wx/choice.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/textctrl.h>
#include <wx/sizer.h>
#include <wx/listctrl.h>
#include <wx/panel.h>
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/icon.h>
#include <wx/button.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <wx/splitter.h>
#include <wx/statusbr.h>
#include <wx/frame.h>

///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// Class MainFrame_g
///////////////////////////////////////////////////////////////////////////////
class MainFrame_g : public wxFrame
{
private:
protected:
    wxSplitterWindow *m_splitter;
    wxPanel *m_panel_left;
    wxChoice *m_adaptor;
    wxTextCtrl *m_filter;
    PacketListCtrl *m_list;
    wxPanel *m_panel_right;
    wxButton *m_start;
    wxButton *m_stop;
    wxButton *m_clear;
    PacketPropGrid *m_prop;
    wxStatusBar *m_status;

public:
    MainFrame_g(wxWindow *parent, wxWindowID id = wxID_ANY, const wxString &title = wxT("Sniff"),
                const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxSize(1000, 600),
                long style = wxDEFAULT_FRAME_STYLE | wxTAB_TRAVERSAL);

    ~MainFrame_g();

    void m_splitterOnIdle(wxIdleEvent &)
    {
        m_splitter->SetSashPosition(675);
        m_splitter->Disconnect(wxEVT_IDLE, wxIdleEventHandler(MainFrame_g::m_splitterOnIdle), NULL,
                               this);
    }
};
