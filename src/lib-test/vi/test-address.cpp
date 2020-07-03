////////////////////////////////////////////////////////////////////////////////
// Name:      test-address.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "../test.h"
#include <boost/version.hpp>
#include <wex/address.h>
#include <wex/macros.h>
#include <wex/managedframe.h>
#include <wex/stc.h>

TEST_SUITE_BEGIN("wex::ex");

TEST_CASE("wex::address")
{
  auto* stc = get_stc();
  stc->set_text("hello0\nhello1\nhello2\nhello3\nhello4\nhello5");

  const int lines = stc->GetLineCount();
  auto*     ex    = new wex::ex(stc);
  wex::data::stc(stc).control(wex::data::control().line(1)).inject();
  ex->marker_add('a');
  wex::data::stc(stc).control(wex::data::control().line(2)).inject();
  ex->marker_add('b');

  REQUIRE(wex::address(ex).get_line() == 0);

  SUBCASE("get_line")
  {
    for (const auto& it : std::vector<std::pair<std::string, int>>{
           {"30", lines},      {"40", lines},    {"-40", 1},   {"3-3", 0},
           {"3-1", 2},         {".", 2},         {".+1", 3},   {"$", lines},
           {"$-2", lines - 2}, {"x", 0},         {"'x", 0},    {"1,3s/x/y", 0},
           {"/2/", 3},         {"?2?", 3},       {"'a", 1},    {"'b", 2},
           {"'b+10", lines},   {"10+'b", lines}, {"'a+'b", 3}, {"'b+'a", 3},
           {"'b-'a", 1}})
    {
      REQUIRE(wex::address(ex, it.first).get_line() == it.second);
    }
  }

  wex::address address(ex, "5");

  SUBCASE("adjust_window")
  {
    REQUIRE(address.adjust_window(""));
    REQUIRE(address.adjust_window("-"));
    REQUIRE(address.adjust_window("+"));
    REQUIRE(address.adjust_window("^"));
    REQUIRE(address.adjust_window("="));
    REQUIRE(address.adjust_window("."));
    REQUIRE(!address.adjust_window("xxx"));
  }

  SUBCASE("append")
  {
    REQUIRE(address.append("appended text"));
    REQUIRE(stc->get_text().find("appended text") != std::string::npos);
  }

  SUBCASE("flags")
  {
    REQUIRE(address.flags_supported(""));
    REQUIRE(address.flags_supported("#"));
    REQUIRE(!address.flags_supported("x"));
  }

  SUBCASE("get_line")
  {
    REQUIRE(wex::address(ex).get_line() == 0);
    REQUIRE(wex::address(ex, "-1").get_line() == 1);
    REQUIRE(wex::address(ex, "1").get_line() == 1);
    REQUIRE(wex::address(ex, "100").get_line() == lines);

    wex::address address2(ex, "'a");
    REQUIRE(address2.get_line() == 1);
    address2.marker_delete();
    REQUIRE(address2.get_line() == 0);
  }

  SUBCASE("insert")
  {
    REQUIRE(address.insert("inserted text"));
    REQUIRE(stc->get_text().find("inserted text") != std::string::npos);
  }

  SUBCASE("marker_add") { REQUIRE(address.marker_add('x')); }

  SUBCASE("marker_delete")
  {
    REQUIRE(!address.marker_delete());
    REQUIRE(address.marker_add('x'));
    REQUIRE(wex::address(ex, "'x").marker_delete());
  }

  SUBCASE("parse")
  {
    REQUIRE(!wex::address(ex, "3").parse(""));
    REQUIRE(wex::address(ex, "3").parse("z"));
    REQUIRE(wex::address(ex, "3").parse("z", "="));
    REQUIRE(!wex::address(ex, "3").parse("z", "P"));
  }

  SUBCASE("put")
  {
    ex->get_macros().set_register('z', "zzzzz");
    REQUIRE(address.put('z'));
    REQUIRE(stc->get_text().find("zzzz") != std::string::npos);
  }

  SUBCASE("read")
  {
    REQUIRE(!address.read("XXXXX"));
    REQUIRE(address.read(wex::test::get_path("test.bin").string()));
#ifdef __UNIX__
#if BOOST_VERSION / 100 % 1000 != 72
    REQUIRE(address.read("!ls"));
#endif
#endif
  }

  SUBCASE("write_line_number") { REQUIRE(address.write_line_number()); }
}

TEST_SUITE_END();
