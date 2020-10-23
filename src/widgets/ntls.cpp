#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif
#include "common.h"
#include "main-frame.h"

class NtlsApp : public wxApp
{
public:
    virtual bool OnInit();
};

wxIMPLEMENT_APP(NtlsApp);

bool NtlsApp::OnInit()
{
    google::InitGoogleLogging(argv[0]);
    char **argv_ = argv;
    gflags::ParseCommandLineFlags(&argc, &argv_, true);
    FLAGS_logtostderr = 1;
    FLAGS_minloglevel = 0;

    MainFrame *frame = new MainFrame();
    frame->Show(true);
    return true;
}
