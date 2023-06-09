////////////////////////////////////////////////////////////////////////////////
// Name:      test-address.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/ex/address.h>
#include <wex/ex/command-parser.h>
#include <wex/ex/ex.h>
#include <wex/ex/macros.h>

#include "test.h"

TEST_CASE("wex::address")
{
  auto* stc = get_stc();
  stc->set_text("hello0\nhello1\nhello2\nhello3\nhello4\nhello5\nhello2");

  wex::data::stc data;
  data.set_stc(stc);
  data.control(wex::data::control().line(1)).inject();
  const auto lines = stc->get_line_count();
  auto*      ex    = new wex::ex(stc);
  ex->marker_add('a');
  data.control(wex::data::control().line(2)).inject();
  ex->marker_add('b');

  SUBCASE("constructor")
  {
    REQUIRE(wex::address(ex).type() == wex::address::IS_SINGLE);
    REQUIRE(wex::address(ex).get_line() == 0);
    REQUIRE(wex::address(ex, 5).get_line() == 5);

    for (const auto& it : std::vector<std::pair<std::string, int>>{
           {"30", lines},      {"40", lines},    {"-40", 1},   {"3-3", 0},
           {"3-1", 2},         {".", 2},         {".+1", 3},   {"$", lines},
           {"$-2", lines - 2}, {"x", 0},         {"'x", 0},    {"1,3s/x/y", 0},
           {"/2/", 3},         {"?2?", 7},       {"'a", 1},    {"'b", 2},
           {"'b+10", lines},   {"10+'b", lines}, {"'a+'b", 3}, {"'b+'a", 3},
           {"'b-'a", 1}})
    {
      CAPTURE(it.first);
      REQUIRE(wex::address(ex, it.first).get_line() == it.second);
    }
  }

  SUBCASE("adjust_window")
  {
    REQUIRE(ex->command(":5z"));
    REQUIRE(ex->command(":5z-"));
    REQUIRE(ex->command(":5z+"));
    REQUIRE(ex->command(":5z^"));
    REQUIRE(ex->command(":5z="));
    REQUIRE(ex->command(":5z."));
    REQUIRE(!ex->command(":5zxxx"));
  }

  SUBCASE("append")
  {
    REQUIRE(ex->command(":5a|appended text"));
    REQUIRE(stc->get_text().contains("appended text"));
  }

  SUBCASE("flags_supported")
  {
    REQUIRE(wex::address::flags_supported(""));
    REQUIRE(wex::address::flags_supported("#"));
    REQUIRE(!wex::address::flags_supported("x"));
  }

  SUBCASE("get_line")
  {
    SUBCASE("marker")
    {
      wex::address address(ex, "'a");
      REQUIRE(address.get_line() == 1);
      address.marker_delete();
      REQUIRE(address.get_line() == 0);
    }

    SUBCASE("number")
    {
      REQUIRE(wex::address(ex).get_line() == 0);
      REQUIRE(wex::address(ex, "-1").get_line() == 1);
      REQUIRE(wex::address(ex, "1").get_line() == 1);
      REQUIRE(wex::address(ex, "100").get_line() == lines);
    }

    SUBCASE("text")
    {
      wex::address address(ex, "/hello2/");
      REQUIRE(address.get_line() == 3);
      REQUIRE(address.get_line(25) == 7);
      REQUIRE(address.get_line() == 3);

      wex::address address2(ex, "/hello3");
      REQUIRE(address2.get_line() == 4);
    }
  }

  wex::address address(ex, "5");

  SUBCASE("insert")
  {
    REQUIRE(ex->command(":5i|inserted text"));
    REQUIRE(stc->get_text().contains("inserted text"));
  }

  SUBCASE("marker_add")
  {
    REQUIRE(address.marker_add('x'));
  }

  SUBCASE("marker_delete")
  {
    REQUIRE(!address.marker_delete());
    REQUIRE(address.marker_add('x'));
    REQUIRE(wex::address(ex, "'x").marker_delete());
  }

  SUBCASE("parse")
  {
    REQUIRE(!wex::address(ex).parse(wex::command_parser(ex, "3")));
    REQUIRE(!wex::address(ex).parse(wex::command_parser(ex, "3zP")));
  }

  SUBCASE("put")
  {
    ex->get_macros().set_register('z', "zzzzz");
    REQUIRE(ex->command(":5pu z"));
    REQUIRE(stc->get_text().contains("zzzz"));
  }

  SUBCASE("read")
  {
    REQUIRE(!ex->command(":5r XXXXX"));
    REQUIRE(ex->command(":5r " + wex::test::get_path("test.bin").string()));
#ifdef __UNIX__
    REQUIRE(ex->command(":5r!ls"));
#endif
  }

  SUBCASE("write_line_number")
  {
    REQUIRE(ex->command(":5="));
  }
}
