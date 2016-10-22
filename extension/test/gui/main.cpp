////////////////////////////////////////////////////////////////////////////////
// Name:      main.cpp
// Purpose:   main for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/extension/managedframe.h>
#include <wx/extension/process.h>
#include <wx/extension/shell.h>

#include "test.h"

class wxExTestManagedFrame : public wxExManagedFrame
{
public:
  wxExTestManagedFrame(wxWindow* parent,
    wxWindowID id,
    const wxString& title)
  : wxExManagedFrame(parent, id, title)
  , m_Process(new wxExProcess()) {;};
  virtual wxExProcess* Process(const std::string& command) override {
    m_Process->Execute(command);
    return m_Process;};
private:
  wxExProcess* m_Process;
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
  
    m_Frame = new wxExTestManagedFrame(
      nullptr, wxID_ANY, wxTheApp->GetAppDisplayName());
    m_Frame->Show();
    
    wxExProcess::PrepareOutput(m_Frame);
    AddPane(m_Frame, wxExProcess::GetShell());
    
    std::vector<wxExStatusBarPane> panes;

    panes.emplace_back(wxExStatusBarPane());
    
    for (int i = 0; i < 15; i++)
    {
      panes.emplace_back(wxExStatusBarPane(wxString::Format("Pane%d", i)));
    }
    
    panes.emplace_back(wxExStatusBarPane("PaneInfo"));
    panes.emplace_back(wxExStatusBarPane("PaneLexer"));
    panes.emplace_back(wxExStatusBarPane("PaneFileType"));
    panes.emplace_back(wxExStatusBarPane("LastPane"));

    m_StatusBar = m_Frame->SetupStatusBar(panes);
    
    m_STC = new wxExSTC(m_Frame);
    AddPane(m_Frame, m_STC);
    
    return true;
  }
  
  static wxExManagedFrame* GetFrame() {return m_Frame;};
  static wxExStatusBar* GetStatusBar() {return m_StatusBar;};
  static wxExSTC* GetSTC() {return m_STC;};
private:
  static wxExTestManagedFrame* m_Frame;
  static wxExStatusBar* m_StatusBar;
  static wxExSTC* m_STC;
}; 

wxExTestManagedFrame* wxExTestGuiApp::m_Frame = nullptr;
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

int main(int argc, char* argv[])
{
  return wxExTestMain(argc, argv, new wxExTestGuiApp(), true);
}
