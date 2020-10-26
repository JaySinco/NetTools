#include "global.h"
#include "main-frame.h"

class NtlsApp : public wxApp
{
public:
    virtual bool OnInit() override;
    virtual int OnExit() override;
};

wxIMPLEMENT_APP(NtlsApp);

bool NtlsApp::OnInit()
{
    wxLog *logger = new wxLogStream(&std::cerr);
    wxLog::SetActiveTarget(logger);

    MainFrame *frame = new MainFrame();
    frame->Show(true);
    return true;
}

int NtlsApp::OnExit()
{
    wxLogMessage("application about to exit.");
    return wxApp::OnExit();
}
