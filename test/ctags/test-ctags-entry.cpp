////////////////////////////////////////////////////////////////////////////////
// Name:      test-ctags-entry.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/ctags/ctags-entry.h>

#include "../test.h"

TEST_CASE("wex::ctags_entry")
{
  wex::ctags_entry filter;

  REQUIRE(!filter.is_active());
  REQUIRE(filter.entry_string(0).empty());

  filter.access("xx");
  REQUIRE(filter.is_active());
  REQUIRE(filter.access() == "xx");

  filter.class_name("yy");
  REQUIRE(filter.class_name() == "yy");

  filter.kind("f");
  REQUIRE(filter.is_function());
  REQUIRE(filter.kind() == "f");

  filter.signature("zz");
  REQUIRE(filter.signature() == "zz");

  REQUIRE(!filter.log().str().empty());

  filter.clear();
  REQUIRE(!filter.is_active());
  REQUIRE(!filter.log().str().empty());
}
