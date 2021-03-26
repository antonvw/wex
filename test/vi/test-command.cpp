////////////////////////////////////////////////////////////////////////////////
// Name:      test-ex-command.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/ex-command.h>
#include <wx/textctrl.h>

#include "test.h"

TEST_SUITE_BEGIN("wex::ex");

TEST_CASE("wex::ex_command")
{
  auto* stc = get_stc();
  stc->set_text("more text\notherline\nother line");

  SUBCASE("constructor command")
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

  SUBCASE("set")
  {
    command.set("G");
    REQUIRE(command.type() == wex::ex_command::type_t::VI);
    REQUIRE(command.str() == "G");

    command.set(":100");
    REQUIRE(command.type() == wex::ex_command::type_t::COMMAND);
    REQUIRE(command.str() == ":");

    command.set(std::string(1, WXK_CONTROL_R) + "=");
    REQUIRE(command.type() == wex::ex_command::type_t::CALC);

    command.set("!");
    REQUIRE(command.type() == wex::ex_command::type_t::ESCAPE);

    command.set("/");
    REQUIRE(command.type() == wex::ex_command::type_t::FIND);

    command.set("?");
    REQUIRE(command.type() == wex::ex_command::type_t::FIND);

    command.set("w");
    REQUIRE(command.type() == wex::ex_command::type_t::VI);
    REQUIRE(command.str() == "w");
  }

  SUBCASE("reset")
  {
    command.set(":100");
    REQUIRE(command.str() == ":");
    command.reset();
    REQUIRE(command.str() == ":");
  }

  SUBCASE("exec")
  {
    command.set("G");
    REQUIRE(command.command() == "G");
    REQUIRE(stc->get_current_line() == 0);
    REQUIRE(command.exec());
    REQUIRE(stc->get_current_line() == 2);
  }

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
    REQUIRE(command.append_exec('g'));
    REQUIRE(stc->get_current_line() == 0);

    command.set(wex::ex_command("dd"));
    REQUIRE(command.command() == "dd");
    REQUIRE(command.get_stc() == stc);
    command.restore(wex::ex_command("ww"));
    REQUIRE(command.command() == "ww");
    command.append("ww");
    REQUIRE(command.command() == "wwww");
    REQUIRE(command.get_stc() == stc);
  }

  SUBCASE("erase")
  {
    command.set("xyz");
    command.no_type();
    command.erase(5);
    REQUIRE(command.command() == "xyz");
    command.erase(0);
    REQUIRE(command.command() == "yz");

    command = wex::ex_command("/xyz");
    REQUIRE(command.command() == "/xyz");
    command.erase(0);
    REQUIRE(command.command() == "/yz");

    command = wex::ex_command("/0123456789");
    REQUIRE(command.command() == "/0123456789");
    command.erase(3, 4);
    REQUIRE(command.command() == "/012789");
  }

  SUBCASE("insert")
  {
    command = wex::ex_command("/0123456789");
    REQUIRE(command.command() == "/0123456789");
    command.insert(1, 'a');
    REQUIRE(command.command() == "/0a123456789");
    command.insert(1, "xyz");
    REQUIRE(command.command() == "/0xyza123456789");
    command.insert(0, '5');
    REQUIRE(command.command() == "/50xyza123456789");
    command.insert(100, 'X');
    REQUIRE(command.command() == "/50xyza123456789X");

    command = wex::ex_command("/");
    command.insert(0, 'x');
    REQUIRE(command.command() == "/x");
  }

  SUBCASE("handle")
  {
    auto* tc = new wxTextCtrl(frame(), wxID_ANY);
    frame()->pane_add(tc);

    tc->SetValue("hello");
    tc->SetInsertionPointEnd();
    command.set("/hello");
    REQUIRE(command.command() == "/hello");

    command.handle(tc, WXK_NONE);
    REQUIRE(command.command() == "/hello");

    command.handle(tc, WXK_BACK);
    REQUIRE(command.command() == "/hell");

    command.handle(tc, WXK_BACK);
    REQUIRE(command.command() == "/hel");

    tc->SetInsertionPoint(0);
    command.handle(tc, WXK_BACK);
    REQUIRE(command.command() == "/hel");
  }
}

TEST_SUITE_END();
