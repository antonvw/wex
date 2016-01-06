////////////////////////////////////////////////////////////////////////////////
// Name:      main.cpp
// Purpose:   main for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016
////////////////////////////////////////////////////////////////////////////////

#define CATCH_CONFIG_RUNNER

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/extension/managedframe.h>
#include <wx/extension/process.h>
#include <wx/extension/shell.h>

#include "test.h"

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
  
    m_Frame = new wxExManagedFrame(nullptr, wxID_ANY, wxTheApp->GetAppDisplayName());
    m_Frame->Show();
    
    wxExProcess::PrepareOutput(m_Frame);
    AddPane(m_Frame, wxExProcess::GetShell());
    
    std::vector<wxExStatusBarPane> panes;

    panes.push_back(wxExStatusBarPane());
    
    for (int i = 0; i < 15; i++)
    {
      panes.push_back(wxExStatusBarPane(wxString::Format("Pane%d", i)));
    }
    
    panes.push_back(wxExStatusBarPane("PaneInfo"));
    panes.push_back(wxExStatusBarPane("PaneLexer"));
    panes.push_back(wxExStatusBarPane("PaneFileType"));
    panes.push_back(wxExStatusBarPane("LastPane"));

    m_StatusBar = m_Frame->SetupStatusBar(panes);
    
    m_STC = new wxExSTC(m_Frame);
    AddPane(m_Frame, m_STC);
    
    return true;
  }
  
  static wxExManagedFrame* GetFrame() {return m_Frame;};
  static wxExStatusBar* GetStatusBar() {return m_StatusBar;};
  static wxExSTC* GetSTC() {return m_STC;};
private:
  static wxExManagedFrame* m_Frame;
  static wxExStatusBar* m_StatusBar;
  static wxExSTC* m_STC;
}; 

wxExManagedFrame* wxExTestGuiApp::m_Frame = nullptr;
wxExStatusBar* wxExTestGuiApp::m_StatusBar = nullptr;
wxExSTC* wxExTestGuiApp::m_STC = nullptr;
  
std::vector<std::pair<std::string, std::string>> GetAbbreviations()
{
  return std::vector<std::pair<std::string, std::string>> {
    {"XX","GREAT"},
    {"YY","WHITE"},
    {"ZZ","SHARK"}};
}
      
std::vector<std::string> GetBuiltinVariables() 
{
  return std::vector<std::string> {
    "Cb", "Cc", "Ce", "Cl", "Created", "Date", "Datetime",
    "Filename", "Fullpath", "Nl", "Path", "Time", "Year"};
}

wxExManagedFrame* GetFrame()
{
  return wxExTestGuiApp::GetFrame();
}
  
wxExStatusBar* GetStatusBar()
{
  return wxExTestGuiApp::GetStatusBar();
}

wxExSTC* GetSTC()
{
  return wxExTestGuiApp::GetSTC();
}
  
void Process(const std::string& str, wxExShell* shell)
{
  for (unsigned i = 0; i < str.length(); ++i)
  {
    shell->ProcessChar(str.at(i));
  }
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

  if (argc < 2 || fails == 0)
  {
    wxGetApp().OnExit();
    exit(fails > 0 ? EXIT_FAILURE: EXIT_SUCCESS);
  }
  
  wxGetApp().OnRun();
  
  return 1;
}
