////////////////////////////////////////////////////////////////////////////////
// Name:      test-marker.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/log-none.h>
#include <wex/syntax/marker.h>
#include <wex/test/test.h>
#include <wx/stc/stc.h>

TEST_CASE("wex::marker")
{
  SUBCASE("default-constructor")
  {
    REQUIRE(!wex::marker().is_ok());
  }

  SUBCASE("constructor")
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
    REQUIRE(wex::marker(5) != wex::marker(5, 2));
    REQUIRE(wex::marker(5) != wex::marker(4));
    REQUIRE(wex::marker(5, 2) == wex::marker(5, 2));
    REQUIRE(wex::marker(5, 1) != wex::marker(5, 2));

    wxStyledTextCtrl s;
    markerx.apply(&s);
  }

  SUBCASE("constructor-xml")
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

  SUBCASE("constructor-xml-invalid")
  {
    wex::log_none      off;
    pugi::xml_document doc;
    REQUIRE(doc.load_string("<marker no = \"x\"></marker>"));
    wex::marker marker(doc.document_element());
    REQUIRE(!marker.is_ok());
  }
}
