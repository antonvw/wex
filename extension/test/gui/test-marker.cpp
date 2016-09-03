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
#include <wx/xml/xml.h>
#include <wx/extension/marker.h>
#include <wx/extension/stc.h>
#include "test.h"

TEST_CASE("wxExMarker","[lexer]")
{
  SECTION("Default constructor")
  {
    REQUIRE( !wxExMarker().IsOk() );
  }
  
  SECTION("Constructor using no, symbol")
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

  SECTION("Constructor xml")
  {
    wxXmlNode xml(wxXML_ELEMENT_NODE, "marker");
    xml.AddAttribute("no", "5");
    new wxXmlNode(&xml, wxXML_TEXT_NODE , "", "mark_character,green,white");

    wxExMarker marker(&xml);
    REQUIRE( marker.GetNo() == 5);
    REQUIRE( marker.GetSymbol() == wxSTC_MARK_CHARACTER);
    REQUIRE( marker.GetForegroundColour() == *wxGREEN);
    REQUIRE( marker.GetBackgroundColour() == *wxWHITE);
    REQUIRE( marker.IsOk());
  }
}
