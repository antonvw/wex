////////////////////////////////////////////////////////////////////////////////
// Name:      test-stream-statistics.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/common/stream-statistics.h>

#include <wex/test/test.h>

TEST_CASE("wex::stream_statistics")
{
  wex::stream_statistics ss;

  REQUIRE(ss.get().empty());
  REQUIRE(ss.get("xx") == 0);

  REQUIRE(ss.inc("xx"));
  REQUIRE(ss.get("xx") == 1);

  REQUIRE(ss.inc("xx", 10) == 11);
  REQUIRE(ss.get("xx") == 11);

  wex::stream_statistics ss2;
  REQUIRE(ss2.get().empty());

  REQUIRE(ss2.inc_actions() == 1);
  REQUIRE(ss2.inc_actions_completed() == 1);

  ss += ss2;

  REQUIRE(ss.get_elements().get_items().size() == 3);
}
