////////////////////////////////////////////////////////////////////////////////
// Name:      test-statistics.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/statistics.h>
#include "../test.h"

TEST_CASE( "wex::statistics" ) 
{
  wex::statistics<long> statistics;
  statistics.Inc("test");
  REQUIRE(statistics.Get("test") == 1);
  statistics.Inc("test");
  REQUIRE(statistics.Get("test") == 2);
  statistics.Set("test", 13);
  REQUIRE(statistics.Get("test") == 13);
  statistics.Dec("test");
  REQUIRE(statistics.Get("test") == 12);
  statistics.Inc("test2");
  REQUIRE(statistics.Get("test2") == 1);
  REQUIRE(statistics.Get().find("test") != std::string::npos);
  REQUIRE(statistics.Get().find("test2") != std::string::npos);

  wex::statistics<long> copy(statistics);
  REQUIRE(copy.Get("test") == 12);
  REQUIRE(copy.Get("test2") == 1);

  statistics.Clear();
  REQUIRE(statistics.GetItems().empty());
  REQUIRE(!copy.GetItems().empty());
}
