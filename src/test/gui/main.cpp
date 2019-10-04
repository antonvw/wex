////////////////////////////////////////////////////////////////////////////////
// Name:      main.cpp
// Purpose:   main for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
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
  namespace test
  {
    class managed_frame : public wex::managed_frame
    {
    public:
      managed_frame()
      : wex::managed_frame()
      , m_Process(new process()) {;};
      process* get_process(const std::string& command) override {
        m_Process->execute(command);
        return m_Process;};
    private:
      process* m_Process;
    };

    class gui_app : public app
    {
    public: 
      int OnExit() override
      {
        remove("test-ex.txt");
        return test::app::OnExit();
      }
      
      bool OnInit() override
      {
        if (!test::app::OnInit())
        {
          return false;
        }
      
        m_Frame = new managed_frame();
        m_StatusBar = m_Frame->setup_statusbar({
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
        m_stc = new stc();

        m_Frame->Show();

        process::prepare_output(m_Frame); // before adding pane
        
        add_pane(m_Frame, m_stc);
        add_pane(m_Frame, process::get_shell());
        
        return true;
      }
      
      static auto* frame() {return m_Frame;};
      static auto* get_statusbar() {return m_StatusBar;};
      static auto* get_stc() {return m_stc;};
    private:
      inline static managed_frame* m_Frame = nullptr;
      inline static statusbar* m_StatusBar = nullptr;
      inline static stc* m_stc = nullptr;
    }; 
  };
};

std::vector<std::pair<std::string, std::string>> get_abbreviations()
{
  return std::vector<std::pair<std::string, std::string>> {
    {"XX","GREAT"},
    {"YY","WHITE"},
    {"ZZ","SHARK"}};
}
      
std::vector<std::string> get_builtin_variables() 
{
  std::vector<std::string> v;

  for (const auto i : *wex::vi_macros::get_variables())
  {
    if (i.second.is_builtin())
    {
       v.push_back(i.first);
    }
  }

  return v;
}

wex::managed_frame* frame()
{
  return wex::test::gui_app::frame();
}
  
wex::statusbar* get_statusbar()
{
  return wex::test::gui_app::get_statusbar();
}

wex::stc* get_stc()
{
  return wex::test::gui_app::get_stc();
}

void process(const std::string& str, wex::shell* shell)
{
  for (unsigned i = 0; i < str.length(); ++i)
  {
    shell->process_char(str.at(i));
  }
}

IMPLEMENT_APP_NO_MAIN(wex::test::gui_app);

int main(int argc, char* argv[])
{
  return wex::test::main(argc, argv, new wex::test::gui_app());
}
