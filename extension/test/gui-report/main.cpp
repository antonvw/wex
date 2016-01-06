////////////////////////////////////////////////////////////////////////////////
// Name:      main.cpp
// Purpose:   main for wxExtension report unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#define CATCH_CONFIG_RUNNER

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
    int style = wxDEFAULT_FRAME_STYLE);

  virtual wxExListView* Activate(
    wxExListView::wxExListType list_type, 
    const wxExLexer* lexer) override;
private:
  wxExListView* m_Report;
};

FrameWithHistory::FrameWithHistory(wxWindow* parent,
  wxWindowID id,
  const wxString& title,
  size_t maxFiles,
  size_t maxProjects,
  int style)
  : wxExFrameWithHistory(parent, id, title, maxFiles, maxProjects, style)
{
  wxExLexer lexer("cpp");
  m_Report = new wxExListView(
    this, 
    wxExListView::LIST_KEYWORD,
    wxID_ANY,
    &lexer);
  AddPane(this, m_Report);
}

wxExListView* FrameWithHistory::Activate(
  wxExListView::wxExListType list_type, 
  const wxExLexer* lexer)
{
  return m_Report;
}

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

int main(int argc, char *argv[])
{
  Catch::Session session; // There must be exactly once instance

  int returnCode = session.applyCommandLine( argc, argv );
  
  if (returnCode != 0 || session.configData().showHelp)
    return returnCode;

  wxApp::SetInstance( new wxExTestGuiApp() );
  wxEntryStart( argc, argv );
  wxGetApp().OnInit();
  
  const int fails = session.run();

  if (argc < 2)
  {
    wxGetApp().OnExit();
    exit(fails > 0 ? EXIT_FAILURE: EXIT_SUCCESS);
  }
  
  wxGetApp().OnRun();
  
  return 1;
}
