////////////////////////////////////////////////////////////////////////////////
// Name:      test-ctags.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/ctags.h>
#include <wex/vi.h>

#include "test.h"

TEST_CASE("wex::ctags")
{
  wex::data::stc data;

  auto* ex = new wex::vi(get_stc());

  SUBCASE("default")
  {
    // default find
    REQUIRE(wex::ctags::find("test_app"));
    REQUIRE(!wex::ctags::find("xest_app"));

    // default auto_complete
    REQUIRE(wex::ctags(ex).auto_complete("test_").starts_with("test_app"));

    // setup a filter using find
    wex::ctags_entry filter;
    REQUIRE(wex::ctags::find("test_app", filter));
    REQUIRE(filter.class_name() == "test_app");
    REQUIRE(filter.kind() == "f");

    // auto_complete using filter should now return member functions
    REQUIRE(wex::ctags(ex).auto_complete("me", filter).starts_with("method"));
    REQUIRE(wex::ctags(ex).auto_complete("he", filter).empty());
  }

  SUBCASE("non_existing_file")
  {
    REQUIRE(wex::ctags::close());
    REQUIRE(!wex::ctags::close());

    wex::ctags::open("xxx");

    REQUIRE(!wex::ctags::find("test_app"));
  }

  SUBCASE("own_file")
  {
    REQUIRE(wex::ctags::close());

    wex::ctags::open("test-ctags");

    REQUIRE(!wex::ctags::find(""));
    REQUIRE(!wex::ctags::next());
    REQUIRE(!wex::ctags::previous());
    REQUIRE(!wex::ctags::find("xxxx"));
    REQUIRE(wex::ctags::find("test_app"));
    REQUIRE(!wex::ctags::next());
    auto* ex = new wex::vi(get_stc());
    REQUIRE(wex::ctags(ex).separator() != ' ');
  }
}
