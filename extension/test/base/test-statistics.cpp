////////////////////////////////////////////////////////////////////////////////
// Name:      test-statistics.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/extension/statistics.h>
#include "../test.h"

TEST_CASE( "wxExStatistics" ) 
{
  wxExStatistics<long> statistics;
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

  wxExStatistics<long> copy(statistics);
  REQUIRE(copy.Get("test") == 12);
  REQUIRE(copy.Get("test2") == 1);

  statistics.Clear();
  REQUIRE(statistics.GetItems().empty());
  REQUIRE(!copy.GetItems().empty());
}
