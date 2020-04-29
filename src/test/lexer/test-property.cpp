////////////////////////////////////////////////////////////////////////////////
// Name:      test-property.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/property.h>
#include <wex/managedframe.h>
#include <wex/stc.h>
#include "../test.h"

TEST_CASE("wex::property")
{
  SUBCASE("Constructor xml")
  {
    REQUIRE( !wex::property().is_ok() );

    pugi::xml_document doc;
    REQUIRE( doc.load_string("<property name = \"fold.comment\">2</property>"));

    wex::property p(doc.document_element());
    REQUIRE( p.name() == "fold.comment");
    REQUIRE( p.value() == "2");
    REQUIRE( p.is_ok());
  }
  
  SUBCASE("Constructor name, value")
  {
    wex::property p("man", "ugly");
    
    REQUIRE( p.is_ok());
    REQUIRE( p.name() == "man");
    REQUIRE( p.value() == "ugly");
    
    p.apply(get_stc());
    REQUIRE( p.is_ok());
    
    p.apply_reset(get_stc());
    REQUIRE( p.is_ok());
    
    p.set("xxx");
    REQUIRE( p.name() == "man");
    REQUIRE( p.value() == "xxx");
  }
}
