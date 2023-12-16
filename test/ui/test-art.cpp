////////////////////////////////////////////////////////////////////////////////
// Name:      test-art.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/factory/defs.h>
#include <wex/test/test.h>
#include <wex/ui/art.h>
#include <wxMaterialDesignArtProvider.hpp>

TEST_CASE("wex::art")
{
  SUBCASE("defaults")
  {
    REQUIRE(!wex::stockart::default_client("xxx"));
    REQUIRE(wex::stockart::default_client(wxART_CLIENT_MATERIAL_ROUND));

    REQUIRE(!wex::stockart::default_colour("xxx"));
    REQUIRE(wex::stockart::default_colour("blue"));
  }

  SUBCASE("default-art-type")
  {
    REQUIRE(!wex::stockart(0).get_bitmap().IsOk());
    REQUIRE(!wex::stockart(wxID_ANY).get_bitmap().IsOk());
    REQUIRE(wex::stockart(wxID_CLEAR).get_bitmap().IsOk());
    REQUIRE(wex::stockart(wxID_NEW).get_bitmap().IsOk());
    REQUIRE(wex::stockart(wxID_OPEN).get_bitmap().IsOk());
    REQUIRE(wex::stockart(wex::ID_CLEAR_FILES).get_bitmap().IsOk());
  }

  SUBCASE("art-type")
  {
    wex::stockart::type(wex::stockart::art_t::MATERIAL);
    REQUIRE(!wex::stockart(wxID_NEW).get_bitmap().IsOk());

    wex::stockart::type(wex::stockart::art_t::STOCK);
    REQUIRE(wex::stockart(wxID_NEW).get_bitmap().IsOk());

    wex::stockart::type(wex::stockart::art_t::USER);
    REQUIRE(!wex::stockart(wxID_NEW).get_bitmap().IsOk());

    wex::stockart::insert({{wxID_NEW, wxART_STOP}});
    REQUIRE(wex::stockart(wxID_NEW).get_bitmap().IsOk());
  }
}
