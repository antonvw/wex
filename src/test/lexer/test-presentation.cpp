////////////////////////////////////////////////////////////////////////////////
// Name:      test-presentation.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/presentation.h>
#include <wex/managedframe.h>
#include <wex/stc.h>
#include "../test.h"

TEST_CASE("wex::presentation")
{
  SUBCASE("Constructor")
  {
    REQUIRE( wex::presentation(wex::presentation::MARKER).name() == "marker" );
    REQUIRE(!wex::presentation(wex::presentation::MARKER).is_ok() );
  }
  
  SUBCASE("Constructor using no, style")
  {
    wex::presentation px(wex::presentation::MARKER, 5, 2);
    wex::presentation py(wex::presentation::MARKER, 7, 5);

    REQUIRE(!wex::presentation(wex::presentation::MARKER, 5).is_ok() );
    REQUIRE( px.is_ok());
    REQUIRE( py.is_ok());
    REQUIRE( px < py );
    REQUIRE( px == px );
    REQUIRE( px != py );
    REQUIRE( wex::presentation(wex::presentation::MARKER, 5) == wex::presentation(wex::presentation::MARKER, 5));
    REQUIRE( wex::presentation(wex::presentation::MARKER, 5) == wex::presentation(wex::presentation::MARKER, 5, 2));
    REQUIRE( wex::presentation(wex::presentation::MARKER, 5) != wex::presentation(wex::presentation::MARKER, 4));
    REQUIRE( wex::presentation(wex::presentation::MARKER, 5, 2) == wex::presentation(wex::presentation::MARKER, 5, 2));
    REQUIRE( wex::presentation(wex::presentation::MARKER, 5, 1) != wex::presentation(wex::presentation::MARKER, 5, 2));
  }
  
  SUBCASE("Constructor xml")
  {
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_string("<marker no = \"5\">indic_box,green</marker>");
    REQUIRE( result );

    wex::presentation p(wex::presentation::MARKER, doc.document_element());
    REQUIRE( p.foreground_colour() == "green");
    REQUIRE( p.number() == 5);
    REQUIRE( p.style() == 6);
    REQUIRE(!p.underlined());
    REQUIRE( p.is_ok());
    
    p.apply(get_stc());
    REQUIRE( p.is_ok());
  }

  SUBCASE("Constructor xml invalid no")
  {
    pugi::xml_document doc;
    REQUIRE( doc.load_string("<marker no = \"x\"></marker>"));
    wex::presentation p(wex::presentation::MARKER, doc.document_element());
    REQUIRE(!p.is_ok());
  }
}
