////////////////////////////////////////////////////////////////////////////////
// Name:      data/test-control.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "../test.h"
#include <wex/control-data.h>

TEST_CASE("wex::control_data")
{
  SUBCASE("Constructor")
  {
    REQUIRE(wex::control_data().col() == wex::DATA_NUMBER_NOT_SET);
    REQUIRE(wex::control_data().col(3).col() == 3);
    REQUIRE(wex::control_data().command("xx").command().command() == "xx");
    REQUIRE(wex::control_data().find("xx").find() == "xx");
    REQUIRE(wex::control_data().find("xx").find_flags() == 0);
    REQUIRE(wex::control_data().find("xx", 1).find_flags() == 1);
    REQUIRE(wex::control_data().line() == wex::DATA_NUMBER_NOT_SET);
    REQUIRE(wex::control_data().line(-1).line() == -1);
    REQUIRE(wex::control_data().line(3).line() == 3);
    REQUIRE(!wex::control_data().is_required());
    REQUIRE(wex::control_data().is_required(true).is_required());
    wex::control_data data(wex::control_data().line(3));
    data.reset();
    REQUIRE(data.line() == wex::DATA_NUMBER_NOT_SET);
    REQUIRE(wex::control_data().validator() == nullptr);
  }

  SUBCASE("inject")
  {
    REQUIRE(!wex::control_data().inject());
    REQUIRE(!wex::control_data().line(1).col(5).inject());
  }

  SUBCASE("others")
  {
    const std::bitset<3> org(3); // 011

    std::bitset<3>    bs;
    wex::control_data data;
    data.flags(org, bs);
    REQUIRE(bs.to_string() == "011");

    data.flags(std::bitset<3>(std::string("100")), bs, wex::control_data::OR);
    REQUIRE(bs.to_string() == "111");

    data.flags(std::bitset<3>(std::string("010")), bs, wex::control_data::INV);
    REQUIRE(bs.to_string() == "101");
  }
}
