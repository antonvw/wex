////////////////////////////////////////////////////////////////////////////////
// Name:      main.cpp
// Purpose:   main for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "test.h"

namespace wex
{
namespace test
{
class stc_app : public app
{
public:
  /// Static methods

  static auto* frame() { return m_frame; }
  static auto* get_stc() { return m_stc; }

private:
  /// Virtual interface
  bool OnInit() override;

  inline static wex::frame* m_frame = nullptr;
  inline static wex::stc*   m_stc   = nullptr;
};
}; // namespace test
}; // namespace wex

IMPLEMENT_APP_NO_MAIN(wex::test::stc_app);

int main(int argc, char* argv[])
{
  return wex::test::main(argc, argv, new wex::test::stc_app());
}

bool wex::test::stc_app::OnInit()
{
  if (!test::app::OnInit())
  {
    return false;
  }

  m_frame = new wex::frame();
  SetTopWindow(m_frame);

  m_stc = new wex::stc();
  m_frame->Show();

  m_frame->pane_add(m_stc);

  return true;
}

wex::frame* frame()
{
  return wex::test::stc_app::frame();
}

wex::stc* get_stc()
{
  return wex::test::stc_app::get_stc();
}
