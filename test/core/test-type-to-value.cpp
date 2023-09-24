////////////////////////////////////////////////////////////////////////////////
// Name:      test-type-to-value.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/type-to-value.h>
#include <wex/test/test.h>

TEST_CASE("wex::type_to_value")
{
  REQUIRE(wex::type_to_value<int>("100").get() == 100);
  REQUIRE(wex::type_to_value<int>("A").get() == 65);
  REQUIRE(wex::type_to_value<int>(100).get() == 100);
  REQUIRE(wex::type_to_value<int>(1).get_string() == "ctrl-A");
  REQUIRE(wex::type_to_value<int>("100").get_string() == "100");
  REQUIRE(wex::type_to_value<int>("xxx").get_string() == "xxx");
  REQUIRE(wex::type_to_value<std::string>("100").get() == "100");
  REQUIRE(wex::type_to_value<std::string>("100").get_string() == "100");
}
