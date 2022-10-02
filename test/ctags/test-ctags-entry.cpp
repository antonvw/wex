////////////////////////////////////////////////////////////////////////////////
// Name:      test-ctags-entry.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/ctags/ctags-entry.h>

#include "test.h"

TEST_CASE("wex::ctags_entry")
{
  wex::ctags_entry::register_image(get_stc());

  wex::ctags_entry filter;

  REQUIRE(!filter.is_active());
  REQUIRE(filter.entry().kind == nullptr);
  REQUIRE(filter.entry_string(0).empty());

  REQUIRE(!filter.is_class_name());
  REQUIRE(!filter.is_define());
  REQUIRE(!filter.is_enumerator());
  REQUIRE(!filter.is_enumeration_name());
  REQUIRE(!filter.is_file_name());
  REQUIRE(!filter.is_file_name());
  REQUIRE(!filter.is_function());
  REQUIRE(!filter.is_function_or_prototype());
  REQUIRE(!filter.is_function_prototype());
  REQUIRE(!filter.is_master());
  REQUIRE(!filter.is_member());
  REQUIRE(!filter.is_structure_name());
  REQUIRE(!filter.is_typedef());
  REQUIRE(!filter.is_union_name());
  REQUIRE(!filter.is_variable());

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
