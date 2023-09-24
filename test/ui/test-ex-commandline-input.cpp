////////////////////////////////////////////////////////////////////////////////
// Name:      test-ex-commandline-input.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/ui/ex-commandline-input.h>
#include <wex/ui/ex-commandline.h>

#include "test.h"

TEST_CASE("wex::ex_commandline_input")
{
  auto* cl = new wex::ex_commandline(frame());
  frame()->pane_add(cl->control());

  SUBCASE("none")
  {
    REQUIRE(
      wex::ex_commandline_input(wex::ex_command::type_t::NONE).get().empty());
    REQUIRE(wex::ex_commandline_input(wex::ex_command::type_t::NONE)
              .values()
              .empty());
    REQUIRE(!wex::ex_commandline_input(wex::ex_command::type_t::NONE)
               .set(WXK_UP, cl->control()));
  }

  SUBCASE("find")
  {
    wex::ex_commandline_input cli(wex::ex_command::type_t::FIND);
    cli.set("one");
    REQUIRE(cli.get() == "one");
    REQUIRE(cli.values().front() == "one");

    const wex::ex_commandline_input::values_t e{};
    cli.set(e);
    REQUIRE(cli.values().empty());

    cli.set(wex::ex_commandline_input::values_t{"find3", "find4", "find5"});
    REQUIRE(cli.get() == "find3");
    REQUIRE(cli.values().size() == 3);

    cl->set_text("hello");
    REQUIRE(cl->get_text() == "hello");

    cli.set(cl);
    REQUIRE(cli.get() == "hello");
    REQUIRE(cli.values().size() == 4);
  }

  SUBCASE("keys")
  {
    wex::ex_commandline_input cli(wex::ex_command::type_t::FIND);
    cli.set(
      wex::ex_commandline_input::values_t{"hello", "find3", "find4", "find5"});

    REQUIRE(cli.set(WXK_HOME));
    REQUIRE(cli.get() == "find5");
    REQUIRE(cli.set(WXK_HOME));
    REQUIRE(cli.get() == "find5");
    REQUIRE(cli.set(WXK_UP));
    REQUIRE(cli.get() == "find5");
    REQUIRE(cli.set(WXK_DOWN));
    REQUIRE(cli.get() == "find4");

    REQUIRE(cli.set(WXK_END));
    REQUIRE(cli.get() == "hello");
    REQUIRE(cli.set(WXK_END));
    REQUIRE(cli.get() == "hello");
    REQUIRE(cli.set(WXK_DOWN));
    REQUIRE(cli.get() == "hello");
    REQUIRE(cli.set(WXK_UP));
    REQUIRE(cli.get() == "find3");
    REQUIRE(cli.set(WXK_PAGEDOWN));
    REQUIRE(cli.get() == "hello");

    REQUIRE(!cli.set(WXK_NONE, cl->control()));

    cli.set(wex::ex_commandline_input::values_t{
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
      REQUIRE(cli.set(key, cl->control()));
    }
  }

  SUBCASE("max")
  {
    auto* cli = new wex::ex_commandline_input(wex::ex_command::type_t::FIND);

    cli->set(wex::ex_commandline_input::values_t{
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
    REQUIRE(cli->values().size() == 12);

    delete cli;

    wex::ex_commandline_input tst(wex::ex_command::type_t::FIND);
    REQUIRE(tst.values().size() == 5);
  }
}
