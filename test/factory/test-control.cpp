////////////////////////////////////////////////////////////////////////////////
// Name:      data/test-control.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/factory/control.h>
#include <wex/test/test.h>

TEST_CASE("wex::data::control")
{
  SECTION("constructor")
  {
    REQUIRE(wex::data::control().col() == wex::data::NUMBER_NOT_SET);
    REQUIRE(wex::data::control().line() == wex::data::NUMBER_NOT_SET);
    REQUIRE(!wex::data::control().is_required());
  }

  SECTION("inject")
  {
    REQUIRE(!wex::data::control().inject());
    REQUIRE(!wex::data::control().line(1).col(5).inject());
  }

  SECTION("others")
  {
    const std::bitset<3> org(3); // 011

    std::bitset<3>     bs;
    wex::data::control data;
    data.flags(org, bs);
    REQUIRE(bs.to_string() == "011");

    data.flags(std::bitset<3>(std::string("100")), bs, wex::data::control::OR);
    REQUIRE(bs.to_string() == "111");

    data.flags(std::bitset<3>(std::string("010")), bs, wex::data::control::INV);
    REQUIRE(bs.to_string() == "101");

    data.flags(std::bitset<3>(std::string("001")), bs, wex::data::control::NOT);
    REQUIRE(bs.to_string() == "100");
  }

  SECTION("reset")
  {
    wex::data::control data(wex::data::control().line(3));
    data.reset();
    REQUIRE(data.line() == wex::data::NUMBER_NOT_SET);
    REQUIRE(wex::data::control().validator() == nullptr);
  }

  SECTION("set")
  {
    REQUIRE(wex::data::control().col(3).col() == 3);
    REQUIRE(wex::data::control().command("xx").command() == "xx");
    wex::data::control control;
    control.is_ctag(true);
    REQUIRE(wex::data::control().is_required(true).is_required());
    REQUIRE(wex::data::control().find("xx").find() == "xx");
    REQUIRE(wex::data::control().find("xx").find_flags() == 0);
    REQUIRE(wex::data::control().find("xx", 1).find_flags() == 1);
    REQUIRE(wex::data::control().line(-1).line() == -1);
    REQUIRE(wex::data::control().line(3).line() == 3);
  }
}
