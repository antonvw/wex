////////////////////////////////////////////////////////////////////////////////
// Name:      test-reflection.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/reflection.h>
#include <wex/test/test.h>

#include <iostream>

TEST_CASE("wex::reflection")
{
  SUBCASE("constructor")
  {
    REQUIRE(wex::reflection({}).log().str().empty());
  }

  SUBCASE("log")
  {
    wex::reflection rfl(
      {REFLECT_ADD("x", std::string()), REFLECT_ADD("y", std::string("yyy"))});

    CAPTURE(rfl.log().str());
    REQUIRE(rfl.log().str().contains("x, y: yyy"));
  }

  SUBCASE("log-skip-empty")
  {
    wex::reflection rfl(
      {REFLECT_ADD("x", std::string()), REFLECT_ADD("y", std::string("yyy"))},
      wex::reflection::log_t::SKIP_EMPTY);

    REQUIRE(rfl.log().str() == "y: yyy\n");
  }
}
