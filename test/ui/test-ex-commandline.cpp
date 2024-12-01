////////////////////////////////////////////////////////////////////////////////
// Name:      test-ex_commandline.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/ui/ex-commandline.h>

#include "test.h"

TEST_CASE("wex::ex_commandline")
{
  auto* cl = new wex::ex_commandline(frame(), ":");
  frame()->pane_add(cl->control());
  cl->control()->Show();

  SUBCASE("constructor")
  {
    REQUIRE(cl->control() != nullptr);
    REQUIRE(cl->stc() == nullptr);
    REQUIRE(cl->get_frame() == frame());
    REQUIRE(wex::ex_commandline(frame()).get_text().empty());
    REQUIRE(cl->get_text().empty());
  }

  SUBCASE("events")
  {
    wxKeyEvent event(wxEVT_KEY_DOWN);

    for (auto id : std::vector<int>{
           WXK_TAB,
           WXK_HOME,
           WXK_ESCAPE,
           WXK_RETURN,
           WXK_PAGEDOWN})
    {
      event.m_keyCode = id;
      cl->on_key_down(event);
    }

    cl->set_text(":this is a line");
    cl->control()->SetFocus();
    event.m_keyCode = WXK_HOME;
    cl->on_key_down(event);
    event.m_keyCode = WXK_END;
    cl->on_key_down(event);
    REQUIRE(cl->control()->get_selected_text().empty());
  }

  SUBCASE("stc")
  {
    auto* stc = get_stc();

    REQUIRE(!cl->set_stc(stc, "xxx"));
    REQUIRE(cl->stc() == stc);
    REQUIRE(cl->set_stc(stc, "/abc"));

    cl->set_stc(nullptr);
    REQUIRE(cl->stc() == nullptr);
    REQUIRE(cl->set_stc(stc, "/abc"));
    REQUIRE(cl->stc() == stc);

    cl->set_stc(stc);
    REQUIRE(cl->stc() == stc);

    for (auto c : std::vector<char>{'a', 'c', 'i'})
    {
      REQUIRE(cl->set_stc(stc, c));
    }

    REQUIRE(!cl->set_stc(stc, 'e'));
  }

  SUBCASE("text")
  {
    cl->set_text("xyz");
    REQUIRE(cl->get_text() == "xyz");

    cl->set_text("abc");
    REQUIRE(cl->get_text() == "abc");

    cl->select_all();
    REQUIRE(cl->control()->get_selected_text() == "abc");
  }

  delete cl;
}
