////////////////////////////////////////////////////////////////////////////////
// Name:      test-textctrl-input.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/textctrl-input.h>
#include <wex/textctrl.h>

#include "test.h"

TEST_CASE("wex::textctrl_input")
{
  auto* tc = new wex::textctrl(frame());
  add_pane(frame(), tc->control());

  REQUIRE(wex::textctrl_input(wex::ex_command::type_t::NONE).get().empty());
  REQUIRE(wex::textctrl_input(wex::ex_command::type_t::NONE).values().empty());
  REQUIRE(!wex::textctrl_input(wex::ex_command::type_t::NONE).set(WXK_UP, tc));

  wex::textctrl_input tci(wex::ex_command::type_t::FIND);
  tci.set("one");
  REQUIRE(tci.get() == "one");
  REQUIRE(tci.values().front() == "one");

  tci.set(std::list<std::string>{"find3", "find4", "find5"});
  REQUIRE(tci.get() == "find3");
  REQUIRE(tci.values().size() == 3);

  tc->set_text("hello");
  REQUIRE(tc->get_text() == "hello");

  tci.set(tc);
  REQUIRE(tci.get() == "hello");
  REQUIRE(tci.values().size() == 4);

  // Test keys.
  REQUIRE(tci.set(WXK_HOME));
  REQUIRE(tci.get() == "hello");
  REQUIRE(tci.set(WXK_END));
  REQUIRE(tci.get() == "find5");
  REQUIRE(tci.set(WXK_HOME));
  REQUIRE(tci.get() == "hello");
  REQUIRE(tci.set(WXK_DOWN));
  REQUIRE(tci.get() == "hello");
  REQUIRE(tci.set(WXK_PAGEDOWN));
  REQUIRE(tci.get() == "hello");

  REQUIRE(!tci.set(WXK_NONE, tc));

  tci.set(std::list<std::string>{
    "1",
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9",
    "10",
    "11",
    "12"});

  for (auto key : std::vector<
         int>{WXK_UP, WXK_DOWN, WXK_HOME, WXK_END, WXK_PAGEUP, WXK_PAGEDOWN})
  {
    REQUIRE(tci.set(key, tc));
  }

  const std::list<std::string> e{};
  tci.set(e);
  REQUIRE(tci.values().empty());
}
