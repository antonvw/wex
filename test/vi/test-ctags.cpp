////////////////////////////////////////////////////////////////////////////////
// Name:      test-ctags.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "../test.h"
#include <wex/ctags.h>
#include <wex/frame.h>
#include <wex/stc.h>

TEST_CASE("wex::ctags")
{
  wex::data::stc data;

  SUBCASE("tags default")
  {
    wex::ex* ex = &get_stc()->get_vi();

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

  SUBCASE("tags non-existing file")
  {
    auto* stc = new wex::stc(std::string("test"), data);
    REQUIRE(wex::ctags::close());
    REQUIRE(!wex::ctags::close());

    wex::ctags::open("xxx");
    wex::test::add_pane(frame(), stc);
    wex::ex* ex = &stc->get_vi();

    REQUIRE(!wex::ctags::find("test_app"));
  }

  SUBCASE("tags own file")
  {
    auto* stc = new wex::stc(std::string("test"), data);
    REQUIRE(wex::ctags::close());

    wex::ctags::open("test-ctags");
    wex::test::add_pane(frame(), stc);
    wex::ex* ex = &stc->get_vi();

    REQUIRE(!wex::ctags::find(""));
    REQUIRE(!wex::ctags::next());
    REQUIRE(!wex::ctags::previous());
    REQUIRE(!wex::ctags::find("xxxx"));
    REQUIRE(wex::ctags::find("test_app"));
    REQUIRE(!wex::ctags::next());
    REQUIRE(wex::ctags(ex).separator() != ' ');
  }
}
