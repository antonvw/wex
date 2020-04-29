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
  
  statistics.inc("test");
  REQUIRE(statistics.get("test") == 1);
  statistics.inc("test");
  REQUIRE(statistics.get("test") == 2);
  statistics.set("test", 13);
  REQUIRE(statistics.get("test") == 13);
  statistics.dec("test");
  REQUIRE(statistics.get("test") == 12);
  statistics.inc("test2");
  REQUIRE(statistics.get("test2") == 1);
  REQUIRE(statistics.get().find("test") != std::string::npos);
  REQUIRE(statistics.get().find("test2") != std::string::npos);

  wex::statistics<long> copy(statistics);
  REQUIRE(copy.get("test") == 12);
  REQUIRE(copy.get("test2") == 1);

  statistics.clear();
  REQUIRE(statistics.get_items().empty());
  REQUIRE(!copy.get_items().empty());
}
