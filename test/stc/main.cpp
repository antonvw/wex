////////////////////////////////////////////////////////////////////////////////
// Name:      main.cpp
// Purpose:   main for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/process.h>

#include "test.h"

namespace wex
{
  namespace test
  {
    class stc : public app
    {
    public:
      /// Static methods

      static auto* frame() { return m_frame; };
      static auto* get_statusbar() { return m_statusbar; };
      static auto* get_stc() { return m_stc; };

      /// Virtual interface
      bool OnInit() override;

    private:
      inline static wex::frame* m_frame     = nullptr;
      inline static statusbar*  m_statusbar = nullptr;
      inline static wex::stc*   m_stc       = nullptr;
    };
  }; // namespace test
};   // namespace wex

IMPLEMENT_APP_NO_MAIN(wex::test::stc);

int main(int argc, char* argv[])
{
  return wex::test::main(argc, argv, new wex::test::stc());
}

bool wex::test::stc::OnInit()
{
  if (!test::app::OnInit())
  {
    return false;
  }

  m_frame     = new wex::frame();
  m_statusbar = m_frame->setup_statusbar(
    {{"Pane0"}, // the order of panes is tested
     {"Pane1"},
     {"Pane2"},
     {"Pane3"},
     {"Pane4"},
     {"PaneInfo"},
     {"PaneLexer"},
     {"PaneMode"},
     {"PaneFileType"},
     {"LastPane"}});
  m_stc = new wex::stc();
  m_frame->Show();

  process::prepare_output(m_frame);

  add_pane(m_frame, m_stc);

  return true;
}

wex::frame* frame()
{
  return wex::test::stc::frame();
}

wex::statusbar* get_statusbar()
{
  return wex::test::stc::get_statusbar();
}

wex::stc* get_stc()
{
  return wex::test::stc::get_stc();
}
