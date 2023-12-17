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
    REQUIRE(!wex::art::default_client("xxx"));
    REQUIRE(wex::art::default_client(wxART_CLIENT_MATERIAL_ROUND));

    REQUIRE(!wex::art::default_colour("xxx"));
    REQUIRE(wex::art::default_colour("blue"));
  }

  SUBCASE("default-art-type")
  {
    REQUIRE(!wex::art(0).get_bitmap().IsOk());
    REQUIRE(!wex::art(wxID_ANY).get_bitmap().IsOk());
    REQUIRE(wex::art(wxID_CLEAR).get_bitmap().IsOk());
    REQUIRE(wex::art(wxID_NEW).get_bitmap().IsOk());
    REQUIRE(wex::art(wxID_OPEN).get_bitmap().IsOk());
    REQUIRE(wex::art(wex::ID_CLEAR_FILES).get_bitmap().IsOk());
  }

  SUBCASE("art-type")
  {
    wex::art::type(wex::art::art_t::MATERIAL);
    REQUIRE(!wex::art(wxID_NEW).get_bitmap().IsOk());

    wex::art::type(wex::art::art_t::STOCK);
    REQUIRE(wex::art(wxID_NEW).get_bitmap().IsOk());

    wex::art::type(wex::art::art_t::USER);
    REQUIRE(!wex::art(wxID_NEW).get_bitmap().IsOk());

    wex::art::insert({{wxID_NEW, wxART_STOP}});
    REQUIRE(wex::art(wxID_NEW).get_bitmap().IsOk());
  }
}
