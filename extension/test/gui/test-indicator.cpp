////////////////////////////////////////////////////////////////////////////////
// Name:      test-indicator.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/indicator.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/stc.h>
#include "test.h"

TEST_CASE("wxExIndicator")
{
  SUBCASE("Default constructor")
  {
    REQUIRE(!wxExIndicator().IsOk() );
  }
  
  SUBCASE("Constructor using no, symbol")
  {
    wxExIndicator indx(5, 2);
    wxExIndicator indy(7, 5);

    REQUIRE(!wxExIndicator(5).IsOk() );
    REQUIRE( indx.IsOk());
    REQUIRE( indy.IsOk());
    REQUIRE( indx < indy );
    REQUIRE( indx == indx );
    REQUIRE( indx != indy );
    REQUIRE( wxExIndicator(5) == wxExIndicator(5));
    REQUIRE( wxExIndicator(5) == wxExIndicator(5, 2));
    REQUIRE( wxExIndicator(5) != wxExIndicator(4));
    REQUIRE( wxExIndicator(5, 2) == wxExIndicator(5, 2));
    REQUIRE( wxExIndicator(5, 1) != wxExIndicator(5, 2));
  }
  
  SUBCASE("Constructor xml")
  {
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_string("<indicator no = \"5\">indic_box,green</indicator>");
    REQUIRE( result );

    wxExIndicator ind(doc.document_element());
    REQUIRE( ind.GetForegroundColour() == "green");
    REQUIRE( ind.GetNo() == 5);
    REQUIRE( ind.GetStyle() == 6);
    REQUIRE(!ind.GetUnder());
    REQUIRE( ind.IsOk());
    
    ind.Apply(GetSTC());
    REQUIRE( ind.IsOk());
  }

  SUBCASE("Constructor xml invalid no")
  {
    pugi::xml_document doc;
    REQUIRE( doc.load_string("<indicator no = \"x\"></indicator>"));
    wxExIndicator ind(doc.document_element());
    REQUIRE(!ind.IsOk());
  }
}
