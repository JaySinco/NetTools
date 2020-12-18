#include "main-frame.h"

class App : public wxApp
{
public:
    virtual bool OnInit() override;
    virtual int OnExit() override;
};

wxIMPLEMENT_APP(App);

bool App::OnInit()
{
    char **argv_ = argv;
    INIT_LOG(argc, argv_);

    MainFrame *frame = new MainFrame;
    frame->Show(true);
    return true;
}

int App::OnExit()
{
    VLOG(1) << "application about to exit.";
    return wxApp::OnExit();
}
