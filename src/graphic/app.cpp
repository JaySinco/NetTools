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
    google::InitGoogleLogging(argv[0]);
    char **argv_ = argv;
    gflags::ParseCommandLineFlags(&argc, &argv_, true);
    FLAGS_logtostderr = 1;
    FLAGS_minloglevel = 0;

    MainFrame *frame = new MainFrame;
    frame->Show(true);
    return true;
}

int App::OnExit()
{
    VLOG(1) << "application about to exit.";
    return wxApp::OnExit();
}
