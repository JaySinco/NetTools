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
    MainFrame *frame = new MainFrame;
    frame->Show(true);
    return true;
}

int App::OnExit()
{
    spdlog::debug("application about to exit.");
    return wxApp::OnExit();
}
