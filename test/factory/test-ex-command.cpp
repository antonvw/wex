////////////////////////////////////////////////////////////////////////////////
// Name:      test-ex-command.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/factory/ex-command.h>

#include "test.h"

// See also stc::test-command

TEST_CASE("wex::ex_command")
{
  auto* stc = new wex::test::stc();
  stc->set_text("more text\notherline\nother line");

  SUBCASE("constructor")
  {
    wex::ex_command command("G");

    REQUIRE(command.command() == "G");
    REQUIRE(command.get_stc() == nullptr);
    REQUIRE(command.type() == wex::ex_command::type_t::VI);

    REQUIRE(!command.exec());
    REQUIRE(stc->get_current_line() == 0);
  }

  SUBCASE("constructor stc")
  {
    wex::ex_command command(stc);

    REQUIRE(command.empty());
    REQUIRE(command.get_stc() == stc);
    REQUIRE(command.type() == wex::ex_command::type_t::NONE);

    SUBCASE("clear")
    {
      command.set("12345");
      REQUIRE(!command.empty());
      command.clear();
      REQUIRE(command.empty());
      REQUIRE(command.str().empty());
    }
  }

  wex::ex_command command(stc);

  SUBCASE("change")
  {
    command.append('g');
    REQUIRE(command.command() == "g");
    command.append('g');
    REQUIRE(command.command() == "gg");
    REQUIRE(command.front() == 'g');
    REQUIRE(command.back() == 'g');
    REQUIRE(command.size() == 2);
    command.pop_back();
    REQUIRE(command.size() == 1);

    command.set(wex::ex_command("dd"));
    REQUIRE(command.command() == "dd");
    REQUIRE(command.get_stc() == stc);
    command.restore(wex::ex_command("ww"));
    REQUIRE(command.command() == "ww");
    command.append("ww");
    REQUIRE(command.command() == "wwww");
    REQUIRE(command.get_stc() == stc);

    REQUIRE(!command.append_exec('w'));
    REQUIRE(command.command() == "wwwww");
  }

  SUBCASE("exec")
  {
    command.set("G");
    REQUIRE(command.command() == "G");
    REQUIRE(stc->get_current_line() == 0);
    REQUIRE(!command.exec());
    REQUIRE(stc->get_current_line() == 0);
  }

  SUBCASE("reset")
  {
    command.set(":100");
    REQUIRE(command.str() == ":");
    command.reset();
    REQUIRE(command.str() == ":");
  }

  SUBCASE("set")
  {
    command.set("G");
    REQUIRE(command.type() == wex::ex_command::type_t::VI);
    REQUIRE(command.str() == "G");

    command.set(":100");
    REQUIRE(command.type() == wex::ex_command::type_t::COMMAND);
    REQUIRE(command.str() == ":");

    stc->visual(false);
    command.set(":100");
    REQUIRE(command.type() == wex::ex_command::type_t::COMMAND_EX);
    REQUIRE(command.str() == ":");
    stc->visual(true);

    command.set(std::string(1, WXK_CONTROL_R) + "=");
    REQUIRE(command.type() == wex::ex_command::type_t::CALC);
    REQUIRE(command.str() == std::string(1, WXK_CONTROL_R) + "=");

    command.set("!");
    REQUIRE(command.type() == wex::ex_command::type_t::ESCAPE);
    REQUIRE(command.str() == "!");

    command.set("/xxx");
    REQUIRE(command.type() == wex::ex_command::type_t::FIND);
    REQUIRE(command.str() == "/");

    command.set("?yyy");
    REQUIRE(command.type() == wex::ex_command::type_t::FIND);
    REQUIRE(command.str() == "?");

    command.set("w");
    REQUIRE(command.type() == wex::ex_command::type_t::VI);
    REQUIRE(command.str() == "w");

    command.set(":" + wex::ex_command::selection_range() + "y");
    REQUIRE(command.type() == wex::ex_command::type_t::COMMAND_RANGE);
    REQUIRE(command.str() == ":" + wex::ex_command::selection_range());

    command.set(":" + wex::ex_command::selection_range() + "!pwd");
    REQUIRE(command.type() == wex::ex_command::type_t::ESCAPE_RANGE);
    REQUIRE(command.str() == ":" + wex::ex_command::selection_range() + "!");
  }
}
