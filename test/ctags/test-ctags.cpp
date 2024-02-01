////////////////////////////////////////////////////////////////////////////////
// Name:      test-ctags.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/log-none.h>
#include <wex/ctags/ctags.h>
#include <wex/ui/frd.h>

#include "test.h"

TEST_CASE("wex::ctags")
{
  SUBCASE("constructor")
  {
    // default auto_complete
    REQUIRE(
      wex::ctags(get_stc()).auto_complete("test_").starts_with("test_app"));

    // setup a filter using find
    wex::ctags_entry filter;
    REQUIRE(wex::ctags::find("test_app", filter));
    REQUIRE(filter.class_name() == "test_app");
    REQUIRE(filter.kind() == "f");

    // auto_complete using filter should now return member functions
    REQUIRE(
      wex::ctags(get_stc()).auto_complete("me", filter).starts_with("method"));
    REQUIRE(wex::ctags(get_stc()).auto_complete("he", filter).empty());

    REQUIRE(wex::ctags(get_stc()).separator() != ' ');
    REQUIRE(wex::ctags::close());
  }

  SUBCASE("existing-ctags")
  {
    REQUIRE(wex::ctags::open("test-ctags"));

    auto* frd = wex::find_replace_data::get();
    frd->set_find_string("find-from-frd");
    REQUIRE(frd->get_find_string() == "find-from-frd");

    REQUIRE(!wex::ctags::find(""));
    REQUIRE(!wex::ctags::next());
    REQUIRE(!wex::ctags::previous());
    REQUIRE(!wex::ctags::find("xxxx"));
    REQUIRE(wex::ctags::find("test_app"));
    REQUIRE(wex::ctags::find("test_app", get_stc()));
    REQUIRE(!wex::ctags::next());
    REQUIRE(!wex::ctags::previous()); // there is only 1 match?

    REQUIRE(frd->get_find_string() == "find-from-frd");

    REQUIRE(wex::ctags::close());
    REQUIRE(!wex::ctags::close());
  }

  SUBCASE("non-existing-ctags")
  {
    wex::log_none off;
    REQUIRE(!wex::ctags::open("xxx"));

    REQUIRE(!wex::ctags::close());
    REQUIRE(!wex::ctags::find("test_app"));
  }
}
