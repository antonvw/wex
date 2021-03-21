////////////////////////////////////////////////////////////////////////////////
// Name:      main.cpp
// Purpose:   main for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "test.h"

namespace wex
{
  namespace test
  {
    class vi : public app
    {
    public:
      /// Static methods

      static auto* frame() { return m_frame; };
      static auto* get_stc() { return m_stc; };

      /// Virtual interface
      bool OnInit() override;

    private:
      inline static wex::frame* m_frame = nullptr;
      inline static stc*        m_stc   = nullptr;
    };
  }; // namespace test
};   // namespace wex

IMPLEMENT_APP_NO_MAIN(wex::test::vi);

int main(int argc, char* argv[])
{
  return wex::test::main(argc, argv, new wex::test::vi());
}

bool wex::test::vi::OnInit()
{
  if (!test::app::OnInit())
  {
    return false;
  }

  m_frame = new wex::frame();
  m_stc   = new stc();
  m_frame->Show();

  add_pane(m_frame, m_stc);

  return true;
}

wex::frame* frame()
{
  return wex::test::vi::frame();
}

wex::stc* get_stc()
{
  return wex::test::vi::get_stc();
}
