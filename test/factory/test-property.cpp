////////////////////////////////////////////////////////////////////////////////
// Name:      test-property.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/factory/property.h>

#include "../test.h"

TEST_CASE("wex::property")
{
  SUBCASE("constructor")
  {
    REQUIRE(!wex::property().is_ok());

    pugi::xml_document doc;
    REQUIRE(doc.load_string("<property name = \"fold.comment\">2</property>"));

    wex::property p(doc.document_element());
    REQUIRE(p.name() == "fold.comment");
    REQUIRE(p.value() == "2");
    REQUIRE(p.is_ok());
  }

  SUBCASE("constructor-2")
  {
    wex::property p("man", "ugly");

    REQUIRE(p.is_ok());
    REQUIRE(p.name() == "man");
    REQUIRE(p.value() == "ugly");

    wxStyledTextCtrl s;
    p.apply(&s);
    REQUIRE(p.is_ok());

    p.apply_reset(&s);
    REQUIRE(p.is_ok());

    p.set("xxx");
    REQUIRE(p.name() == "man");
    REQUIRE(p.value() == "xxx");
  }
}
