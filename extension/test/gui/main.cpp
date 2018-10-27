////////////////////////////////////////////////////////////////////////////////
// Name:      main.cpp
// Purpose:   main for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wex/managedframe.h>
#include <wex/process.h>
#include <wex/shell.h>
#include <wex/vi-macros.h>

#include "test.h"

namespace wex
{
  class test_managed_frame : public managed_frame
  {
  public:
    test_managed_frame()
    : managed_frame()
    , m_Process(new process()) {;};
    virtual process* Process(const std::string& command) override {
      m_Process->Execute(command);
      return m_Process;};
  private:
    process* m_Process;
  };

  class gui_test_app : public test_app
  {
  public: 
    gui_test_app() : test_app() {;};
    
    virtual int OnExit() override
    {
      remove("test-ex.txt");
      return test_app::OnExit();
    }
    
    virtual bool OnInit() override
    {
      if (!test_app::OnInit())
      {
        return false;
      }
    
      m_Frame = new test_managed_frame();
      m_StatusBar = m_Frame->SetupStatusBar({
        {"Pane0"}, // the order of panes is tested
        {"Pane1"},
        {"Pane2"},
        {"Pane3"},
        {"Pane4"},
        {"PaneInfo"},
        {"PaneLexer"},
        {"PaneMode"},
        {"PaneFileType"},
        {"LastPane"}});
      m_STC = new stc();

      m_Frame->Show();

      process::PrepareOutput(m_Frame); // before adding pane
      
      AddPane(m_Frame, m_STC);
      AddPane(m_Frame, process::GetShell());
      
      return true;
    }
    
    static auto* GetFrame() {return m_Frame;};
    static auto* GetStatusBar() {return m_StatusBar;};
    static auto* GetSTC() {return m_STC;};
  private:
    static test_managed_frame* m_Frame;
    static statusbar* m_StatusBar;
    static stc* m_STC;
  }; 
};

wex::test_managed_frame* wex::gui_test_app::m_Frame = nullptr;
wex::statusbar* wex::gui_test_app::m_StatusBar = nullptr;
wex::stc* wex::gui_test_app::m_STC = nullptr;
  
std::vector<std::pair<std::string, std::string>> GetAbbreviations()
{
  return std::vector<std::pair<std::string, std::string>> {
    {"XX","GREAT"},
    {"YY","WHITE"},
    {"ZZ","SHARK"}};
}
      
std::vector<std::string> GetBuiltinVariables() 
{
  std::vector<std::string> v;

  for (const auto i : wex::vi_macros::GetVariables())
  {
    if (i.second.IsBuiltIn())
    {
       v.push_back(i.first);
    }
  }

  return v;
}

wex::managed_frame* GetFrame()
{
  return wex::gui_test_app::GetFrame();
}
  
wex::statusbar* GetStatusBar()
{
  return wex::gui_test_app::GetStatusBar();
}

wex::stc* GetSTC()
{
  return wex::gui_test_app::GetSTC();
}
  
void Process(const std::string& str, wex::shell* shell)
{
  for (unsigned i = 0; i < str.length(); ++i)
  {
    shell->ProcessChar(str.at(i));
  }
}

IMPLEMENT_APP_NO_MAIN(wex::gui_test_app);

int main(int argc, char* argv[])
{
  return wex::testmain(argc, argv, new wex::gui_test_app());
}
