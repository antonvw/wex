////////////////////////////////////////////////////////////////////////////////
// Name:      test-marker.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/marker.h>
#include <wx/extension/stc.h>
#include "test.h"

TEST_CASE("wxExMarker")
{
  SUBCASE("Default constructor")
  {
    REQUIRE( !wxExMarker().IsOk() );
  }
  
  SUBCASE("Constructor using no, symbol")
  {
    REQUIRE(!wxExMarker(1, 100).IsOk());
    REQUIRE( wxExMarker(1, wxSTC_MARK_CHARACTER).IsOk());
    REQUIRE( wxExMarker(1, wxSTC_MARK_CHARACTER + 100).IsOk());
    REQUIRE(!wxExMarker(1, wxSTC_MARK_CHARACTER + 300).IsOk());
    
    wxExMarker markerx(5, 2);
    wxExMarker markery(7, 5);
    
    REQUIRE(!wxExMarker(0).IsOk());
    REQUIRE( markerx.IsOk());
    REQUIRE( markery.IsOk());
    REQUIRE( markerx < markery );
    REQUIRE( markerx == markerx );
    REQUIRE( wxExMarker(5) == wxExMarker(5));
    REQUIRE( wxExMarker(5) == wxExMarker(5, 2));
    REQUIRE( wxExMarker(5) != wxExMarker(4));
    REQUIRE( wxExMarker(5, 2) == wxExMarker(5, 2));
    REQUIRE( wxExMarker(5, 1) != wxExMarker(5, 2));
    
    markerx.Apply(GetSTC());
  }

  SUBCASE("Constructor xml")
  {
    pugi::xml_document doc;
    REQUIRE( doc.load_string("<marker no = \"5\">mark_character,green,white</marker>"));

    wxExMarker marker(doc.document_element());
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
    wxExMarker marker(doc.document_element());
    REQUIRE(!marker.IsOk());
  }
}
