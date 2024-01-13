////////////////////////////////////////////////////////////////////////////////
// Name:      main.cpp
// Purpose:   main for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2024
////////////////////////////////////////////////////////////////////////////////

#include "test.h"

namespace wex
{
namespace test
{
class factory : public app
{
public:
  // Static methods

  static auto* frame() { return m_frame; }

private:
  // Virtual interface
  bool OnInit() override;

  inline static wex::factory::frame* m_frame = nullptr;
};
}; // namespace test
}; // namespace wex

IMPLEMENT_APP_NO_MAIN(wex::test::app);

int main(int argc, char* argv[])
{
  return wex::test::main(argc, argv, new wex::test::factory());
}

bool wex::test::factory::OnInit()
{
  if (!wex::app::OnInit() || !on_init(this))
  {
    return false;
  }

  m_frame = new wex::factory::frame(nullptr, -1, "factory test");

  m_frame->Show();

  SetTopWindow(m_frame);

  return true;
}

wex::factory::frame* frame()
{
  return wex::test::factory::frame();
}
