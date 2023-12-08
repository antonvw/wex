////////////////////////////////////////////////////////////////////////////////
// Name:      test-ex_commandline.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/ui/ex-commandline.h>

#include "test.h"

TEST_CASE("wex::ex_commandline")
{
  auto* cl = new wex::ex_commandline(frame());

  REQUIRE(cl->control() != nullptr);
  REQUIRE(cl->stc() == nullptr);

  frame()->pane_add(cl->control());

  REQUIRE(cl->stc() == nullptr);
  REQUIRE(cl->get_frame() == frame());
  REQUIRE(cl->get_text().empty());

  cl->set_text("xyz");
  REQUIRE(cl->get_text() == "xyz");

  cl->set_text("abc");
  REQUIRE(cl->get_text() == "abc");

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
}
