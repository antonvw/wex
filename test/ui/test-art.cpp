////////////////////////////////////////////////////////////////////////////////
// Name:      test-art.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/test/test.h>
#include <wex/ui/art.h>

TEST_CASE("wex::art")
{
  REQUIRE(!wex::stockart(0).get_bitmap().IsOk());
  REQUIRE(!wex::stockart(wxID_ANY).get_bitmap().IsOk());
  REQUIRE(wex::stockart(wxID_NEW).get_bitmap().IsOk());
  REQUIRE(wex::stockart(wxID_OPEN).get_bitmap().IsOk());
}
