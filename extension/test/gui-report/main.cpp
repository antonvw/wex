////////////////////////////////////////////////////////////////////////////////
// Name:      main.cpp
// Purpose:   main for wxExtension report unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "test.h"

class FrameWithHistory : public wxExFrameWithHistory
{
public:
  FrameWithHistory(wxWindow* parent,
    wxWindowID id,
    const wxString& title,
    size_t maxFiles = 9,
    size_t maxProjects = 0,
    int style = wxDEFAULT_FRAME_STYLE) 
    : wxExFrameWithHistory(parent, id, title, maxFiles, maxProjects, style) {
      wxExLexer lexer("cpp");
      m_Report = new wxExListView(
        this, 
        wxExListView::LIST_KEYWORD,
        wxID_ANY,
        &lexer);
      AddPane(this, m_Report);};

  virtual wxExListView* Activate(
    wxExListView::wxExListType list_type, 
    const wxExLexer* lexer) override {
    return m_Report;};
private:
  wxExListView* m_Report;
};

class wxExTestGuiApp : public wxExTestApp
{
public: 
  wxExTestGuiApp() : wxExTestApp() {;};

  bool OnInit() 
  {
    if (!wxExTestApp::OnInit())
    {
      return false;
    }
  
    m_Frame = new FrameWithHistory(nullptr, wxID_ANY, wxTheApp->GetAppDisplayName());
    m_Frame->Show();
    
    return true;
  }
  
  static wxExFrameWithHistory* GetFrame() {return m_Frame;};
private:
  static wxExFrameWithHistory* m_Frame;
}; 

wxExFrameWithHistory* wxExTestGuiApp::m_Frame = nullptr;

wxExFrameWithHistory* GetFrame()
{
  return wxExTestGuiApp::GetFrame();
}
  
const wxString GetProject()
{
  return "test-rep.prj";
}
  
IMPLEMENT_APP_NO_MAIN(wxExTestGuiApp);

int main(int argc, char* argv[])
{
  return wxExTestMain(argc, argv, new wxExTestGuiApp(), true);
}
