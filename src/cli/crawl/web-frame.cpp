#include "web-frame.h"

WebFrame::WebFrame() : WebFrame_g(nullptr) { m_browser->LoadURL("https://www.github.com"); }

void WebFrame::OnUrl(wxCommandEvent &event) {}
