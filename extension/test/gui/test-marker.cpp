////////////////////////////////////////////////////////////////////////////////
// Name:      test-marker.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/marker.h>
#include <wx/extension/stc.h>
#include "test.h"

TEST_CASE("wex::marker")
{
  SUBCASE("Default constructor")
  {
    REQUIRE( !wex::marker().IsOk() );
  }
  
  SUBCASE("Constructor using no, symbol")
  {
    REQUIRE(!wex::marker(1, 100).IsOk());
    REQUIRE( wex::marker(1, wxSTC_MARK_CHARACTER).IsOk());
    REQUIRE( wex::marker(1, wxSTC_MARK_CHARACTER + 100).IsOk());
    REQUIRE(!wex::marker(1, wxSTC_MARK_CHARACTER + 300).IsOk());
    
    wex::marker markerx(5, 2);
    wex::marker markery(7, 5);
    
    REQUIRE(!wex::marker(0).IsOk());
    REQUIRE( markerx.IsOk());
    REQUIRE( markery.IsOk());
    REQUIRE( markerx < markery );
    REQUIRE( markerx == markerx );
    REQUIRE( wex::marker(5) == wex::marker(5));
    REQUIRE( wex::marker(5) == wex::marker(5, 2));
    REQUIRE( wex::marker(5) != wex::marker(4));
    REQUIRE( wex::marker(5, 2) == wex::marker(5, 2));
    REQUIRE( wex::marker(5, 1) != wex::marker(5, 2));
    
    markerx.Apply(GetSTC());
  }

  SUBCASE("Constructor xml")
  {
    pugi::xml_document doc;
    REQUIRE( doc.load_string("<marker no = \"5\">mark_character,green,white</marker>"));

    wex::marker marker(doc.document_element());
    REQUIRE( marker.GetNo() == 5);
    REQUIRE( marker.GetSymbol() == wxSTC_MARK_CHARACTER);
    REQUIRE( marker.GetForegroundColour() == "green");
    REQUIRE( marker.GetBackgroundColour() == "white");
    REQUIRE( marker.IsOk());
  }
    
  SUBCASE("Constructor xml invalid no")
  {
    pugi::xml_document doc;
    REQUIRE( doc.load_string("<marker no = \"x\"></marker>"));
    wex::marker marker(doc.document_element());
    REQUIRE(!marker.IsOk());
  }
}
