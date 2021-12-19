////////////////////////////////////////////////////////////////////////////////
// Name:      main.cpp
// Purpose:   main for wex del unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "test.h"

namespace wex
{
namespace test
{
class frame : public del::frame
{
public:
  frame()
    : del::frame()
  {
  }

  void more_coverage() { file_history_list(); }

private:
  del::listview*
  activate(data::listview::type_t listview_type, const lexer* lexer) override
  {
    if (m_lv == nullptr)
    {
      m_lv = new del::listview(data::listview().type(data::listview::FIND));
      pane_add(m_lv);
    }

    return m_lv;
  }

private:
  del::listview* m_lv{nullptr};
};

class del : public app
{
public:
  bool OnInit() override
  {
    if (!test::app::OnInit())
    {
      return false;
    }

    m_frame = new test::frame();
    m_frame->more_coverage();
    m_frame->Show();

    SetTopWindow(m_frame);

    m_stc = new wex::stc();

    return true;
  }

  static auto* stc() { return m_stc; }
  static auto* frame() { return m_frame; }

private:
  inline static wex::stc*    m_stc   = nullptr;
  inline static test::frame* m_frame = nullptr;
};
}; // namespace test
}; // namespace wex

IMPLEMENT_APP_NO_MAIN(wex::test::del);

wex::del::frame* del_frame()
{
  return wex::test::del::frame();
}

const wex::path get_project()
{
  return wex::path("test-del.prj");
}

int main(int argc, char* argv[])
{
  return wex::test::main(argc, argv, new wex::test::del());
}

wex::stc* get_stc()
{
  return wex::test::del::stc();
}
