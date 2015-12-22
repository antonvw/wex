////////////////////////////////////////////////////////////////////////////////
// Name:      test-statistics.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/extension/textfile.h>
#include "../catch.hpp"
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
  REQUIRE(statistics.Get().Contains("test"));
  REQUIRE(statistics.Get().Contains("test2"));

  wxExStatistics<long> copy(statistics);
  REQUIRE(copy.Get("test") == 12);
  REQUIRE(copy.Get("test2") == 1);

  statistics.Clear();
  REQUIRE(statistics.GetItems().empty());
  REQUIRE(!copy.GetItems().empty());
}
