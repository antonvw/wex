////////////////////////////////////////////////////////////////////////////////
// Name:      main.cpp
// Purpose:   main for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "test.h"

class ex_stc : public wex::factory::stc
{
public:
  explicit ex_stc(wxFrame* parent)
  {
    Create(parent, -1);
    Show();
  };

private:
  const wex::path& path() const override { return m_path; };
  wex::path        m_path;
};

namespace wex
{
namespace test
{
class ex : public app
{
public:
  /// Static methods

  static auto* frame() { return m_frame; }
  static auto* get_stc() { return m_stc; }

  /// Virtual interface
  bool OnInit() override;

private:
  inline static wex::frame* m_frame = nullptr;
  inline static ex_stc*     m_stc   = nullptr;
};
}; // namespace test
}; // namespace wex

IMPLEMENT_APP_NO_MAIN(wex::test::ex);

int main(int argc, char* argv[])
{
  return wex::test::main(argc, argv, new wex::test::ex());
}

bool wex::test::ex::OnInit()
{
  if (!test::app::OnInit())
  {
    return false;
  }

  m_frame = new wex::frame();
  m_stc   = new ex_stc(m_frame);
  m_frame->Show();
  m_frame->pane_add(m_stc);

  SetTopWindow(m_frame);

  return true;
}

wex::frame* frame()
{
  return wex::test::ex::frame();
}

wex::factory::stc* get_stc()
{
  return wex::test::ex::get_stc();
}
