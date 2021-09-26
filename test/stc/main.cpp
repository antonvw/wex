////////////////////////////////////////////////////////////////////////////////
// Name:      main.cpp
// Purpose:   main for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/stc/process.h>

#include "test.h"

namespace wex
{
namespace test
{
class stc : public app
{
public:
  /// Static methods

  static auto* frame() { return m_frame; }
  static auto* get_stc() { return m_stc; }

  /// Virtual interface
  bool OnInit() override;

private:
  inline static wex::frame* m_frame = nullptr;
  inline static wex::stc*   m_stc   = nullptr;
};
}; // namespace test
}; // namespace wex

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

  m_frame = new wex::frame();
  SetTopWindow(m_frame);

  m_stc = new wex::stc();
  m_frame->Show();

  process::prepare_output(m_frame);

  m_frame->pane_add(m_stc);

  return true;
}

wex::frame* frame()
{
  return wex::test::stc::frame();
}

wex::stc* get_stc()
{
  return wex::test::stc::get_stc();
}
