////////////////////////////////////////////////////////////////////////////////
// Name:      main.cpp
// Purpose:   main for wex del unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/del/app.h>
#include <wex/test/doctest.h>

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

class del
  : public wex::del::app
  , public doctester
{
public:
  bool OnInit() final
  {
    if (!wex::del::app::OnInit() || !on_init(this))
    {
      return false;
    }

    m_frame = new test::frame();
    m_frame->more_coverage();

    SetTopWindow(m_frame);

    m_stc = new wex::stc();
    m_frame->pane_add(m_stc);
    m_frame->Show();

    return true;
  }

  int OnRun() final
  {
    on_run(this);

    return wex::del::app::OnRun();
  }

  static auto* frame() { return m_frame; }
  static auto* stc() { return m_stc; }

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
  auto* app = new wex::test::del();

  return app->use_context(app, argc, argv) && app->OnInit() && app->OnRun() ?
           1 :
           0;
}

wex::stc* get_stc()
{
  return wex::test::del::stc();
}
