////////////////////////////////////////////////////////////////////////////////
// Name:      data/test-window.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/factory/window.h>

#include <wex/test/test.h>

TEST_CASE("wex::data::window")
{
  REQUIRE(wex::data::window().id() == wxID_ANY);
  REQUIRE(wex::data::window().name().empty());
  REQUIRE(wex::data::window().name("xxx").name() == "xxx");
}
