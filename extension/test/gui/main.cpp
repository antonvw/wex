////////////////////////////////////////////////////////////////////////////////
// Name:      main.cpp
// Purpose:   main for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017
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
  wxExTestManagedFrame()
  : wxExManagedFrame()
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
  
  virtual void ExitMainLoop() override 
  {
    wxExTestApp::ExitMainLoop();
  }
  
  virtual bool OnInit() override
  {
    if (!wxExTestApp::OnInit())
    {
      return false;
    }
  
    m_Frame = new wxExTestManagedFrame();
    m_StatusBar = m_Frame->SetupStatusBar({
      {},
      {"Pane0"}, // the order of panes is tested
      {"Pane1"},
      {"Pane2"},
      {"Pane3"},
      {"Pane4"},
      {"PaneInfo"},
      {"PaneLexer"},
      {"PaneFileType"},
      {"LastPane"}});
    m_STC = new wxExSTC();

    m_Frame->Show();

    wxExProcess::PrepareOutput(m_Frame); // before adding pane
    
    AddPane(m_Frame, m_STC);
    AddPane(m_Frame, wxExProcess::GetShell());
    
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
