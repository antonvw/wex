////////////////////////////////////////////////////////////////////////////////
// Name:      test-marker.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "../test.h"
#include <wex/marker.h>
#include <wx/stc/stc.h>

TEST_CASE("wex::marker")
{
  SUBCASE("Default constructor") { REQUIRE(!wex::marker().is_ok()); }

  SUBCASE("Constructor using no, symbol")
  {
    REQUIRE(!wex::marker(1, 100).is_ok());
    REQUIRE(wex::marker(1, wxSTC_MARK_CHARACTER).is_ok());
    REQUIRE(wex::marker(1, wxSTC_MARK_CHARACTER + 100).is_ok());
    REQUIRE(!wex::marker(1, wxSTC_MARK_CHARACTER + 300).is_ok());

    wex::marker markerx(5, 2);
    wex::marker markery(7, 5);

    REQUIRE(!wex::marker(0).is_ok());
    REQUIRE(markerx.is_ok());
    REQUIRE(markery.is_ok());
    REQUIRE(markerx < markery);
    REQUIRE(markerx == markerx);
    REQUIRE(wex::marker(5) == wex::marker(5));
    REQUIRE(wex::marker(5) == wex::marker(5, 2));
    REQUIRE(wex::marker(5) != wex::marker(4));
    REQUIRE(wex::marker(5, 2) == wex::marker(5, 2));
    REQUIRE(wex::marker(5, 1) != wex::marker(5, 2));

    wxStyledTextCtrl s;
    markerx.apply(&s);
  }

  SUBCASE("Constructor xml")
  {
    pugi::xml_document doc;
    REQUIRE(doc.load_string(
      "<marker no = \"5\">mark_character,green,white</marker>"));

    wex::marker marker(doc.document_element());
    REQUIRE(marker.number() == 5);
    REQUIRE(marker.symbol() == wxSTC_MARK_CHARACTER);
    REQUIRE(marker.foreground_colour() == "green");
    REQUIRE(marker.background_colour() == "white");
    REQUIRE(marker.is_ok());
  }

  SUBCASE("Constructor xml invalid no")
  {
    pugi::xml_document doc;
    REQUIRE(doc.load_string("<marker no = \"x\"></marker>"));
    wex::marker marker(doc.document_element());
    REQUIRE(!marker.is_ok());
  }
}
