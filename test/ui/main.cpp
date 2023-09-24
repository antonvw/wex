////////////////////////////////////////////////////////////////////////////////
// Name:      main.cpp
// Purpose:   main for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "test.h"

wex::test::ui_stc::ui_stc(const wex::data::stc& data)
  : wex::syntax::stc(data.window())
{
  Show();
}

wex::file& wex::test::ui_stc::get_file()
{
  return m_file;
}

namespace wex
{
namespace test
{
class ui : public app
{
public:
  /// Static methods

  static auto* frame() { return m_frame; }
  static auto* get_statusbar() { return m_statusbar; }
  static auto* get_stc() { return m_stc; }

private:
  /// Virtual interface
  bool OnInit() override;

  inline static wex::frame*  m_frame     = nullptr;
  inline static statusbar*   m_statusbar = nullptr;
  inline static syntax::stc* m_stc       = nullptr;
};
}; // namespace test
}; // namespace wex

IMPLEMENT_APP_NO_MAIN(wex::test::ui);

int main(int argc, char* argv[])
{
  return wex::test::main(argc, argv, new wex::test::ui());
}

bool wex::test::ui::OnInit()
{
  if (!wex::app::OnInit() || !on_init(this))
  {
    return false;
  }

  m_frame     = new wex::frame();
  m_statusbar = m_frame->setup_statusbar({{}});

  wex::data::window data;
  data.parent(m_frame);
  m_stc = new ui_stc(data);
  m_frame->Show();
  m_frame->pane_add(m_stc);

  SetTopWindow(m_frame);

  return true;
}

wex::frame* frame()
{
  return wex::test::ui::frame();
}

wex::statusbar* get_statusbar()
{
  return wex::test::ui::get_statusbar();
}

wex::syntax::stc* get_stc()
{
  return wex::test::ui::get_stc();
}
