////////////////////////////////////////////////////////////////////////////////
// Name:      test-indicator.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/indicator.h>
#include <wex/managedframe.h>
#include <wex/stc.h>
#include "test.h"

TEST_CASE("wex::indicator")
{
  SUBCASE("Default constructor")
  {
    REQUIRE(!wex::indicator().is_ok() );
  }
  
  SUBCASE("Constructor using no, symbol")
  {
    wex::indicator indx(5, 2);
    wex::indicator indy(7, 5);

    REQUIRE(!wex::indicator(5).is_ok() );
    REQUIRE( indx.is_ok());
    REQUIRE( indy.is_ok());
    REQUIRE( indx < indy );
    REQUIRE( indx == indx );
    REQUIRE( indx != indy );
    REQUIRE( wex::indicator(5) == wex::indicator(5));
    REQUIRE( wex::indicator(5) == wex::indicator(5, 2));
    REQUIRE( wex::indicator(5) != wex::indicator(4));
    REQUIRE( wex::indicator(5, 2) == wex::indicator(5, 2));
    REQUIRE( wex::indicator(5, 1) != wex::indicator(5, 2));
  }
  
  SUBCASE("Constructor xml")
  {
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_string("<indicator no = \"5\">indic_box,green</indicator>");
    REQUIRE( result );

    wex::indicator ind(doc.document_element());
    REQUIRE( ind.GetForegroundColour() == "green");
    REQUIRE( ind.GetNo() == 5);
    REQUIRE( ind.GetStyle() == 6);
    REQUIRE(!ind.GetUnder());
    REQUIRE( ind.is_ok());
    
    ind.Apply(GetSTC());
    REQUIRE( ind.is_ok());
  }

  SUBCASE("Constructor xml invalid no")
  {
    pugi::xml_document doc;
    REQUIRE( doc.load_string("<indicator no = \"x\"></indicator>"));
    wex::indicator ind(doc.document_element());
    REQUIRE(!ind.is_ok());
  }
}
