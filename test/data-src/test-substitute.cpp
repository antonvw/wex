////////////////////////////////////////////////////////////////////////////////
// Name:      data/test-substitute.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/data/substitute.h>

#include <wex/test/test.h>

TEST_CASE("wex::data::substitute")
{
  SUBCASE("constructor")
  {
    wex::data::substitute sub;

    REQUIRE(sub.pattern().empty());
    REQUIRE(sub.replacement().empty());
    REQUIRE(!sub.is_confirmed());
    REQUIRE(!sub.is_global());
    REQUIRE(!sub.is_ignore_case());
  }

  SUBCASE("set")
  {
    wex::data::substitute sub;
    REQUIRE(!sub.set(""));
    REQUIRE(!sub.set("x"));

    REQUIRE(sub.set("s/xx/yy/c"));
    REQUIRE(sub.pattern() == "xx");
    REQUIRE(sub.replacement() == "yy");
    REQUIRE(sub.is_confirmed());

    REQUIRE(sub.set("s/x/y/g"));
    REQUIRE(!sub.is_ignore_case());
    REQUIRE(sub.is_global());

    REQUIRE(sub.set("s/x/y/i"));
    REQUIRE(sub.is_ignore_case());
    REQUIRE(!sub.is_confirmed());

    REQUIRE(sub.set("s/x/y/cgi"));
    REQUIRE(sub.is_confirmed());
    REQUIRE(sub.is_global());
    REQUIRE(sub.is_ignore_case());

    REQUIRE(sub.set("s/x/~"));
    REQUIRE(sub.pattern() == "x");
    REQUIRE(sub.replacement() == "y");

    REQUIRE(sub.set("s/~"));
    REQUIRE(sub.pattern() == "x");
    REQUIRE(sub.replacement() == "y");
  }

  SUBCASE("set_global")
  {
    wex::data::substitute sub;
    REQUIRE(!sub.set_global("xxyy"));
    REQUIRE(!sub.set_global("u/xx/yy"));
    REQUIRE(sub.set_global("g/xx/yy"));
    REQUIRE(!sub.is_inverse());
    REQUIRE(sub.set_global("v/xx/yy"));
    REQUIRE(sub.pattern() == "xx");
    REQUIRE(sub.commands() == "yy");
    REQUIRE(sub.is_inverse());
  }

  SUBCASE("set_options")
  {
    wex::data::substitute sub;
    sub.set_options("cgi");

    REQUIRE(sub.is_confirmed());
    REQUIRE(sub.is_ignore_case());
    REQUIRE(sub.is_global());
  }
}
