////////////////////////////////////////////////////////////////////////////////
// Name:      test-property.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/property.h>
#include <wex/managedframe.h>
#include <wex/stc.h>
#include "test.h"

TEST_CASE("wex::property")
{
  wex::property inv;
  REQUIRE( !inv.is_ok() );
  
  wex::property prop("man", "ugly");
  
  REQUIRE( prop.is_ok());
  REQUIRE( prop.GetName() == "man");
  REQUIRE( prop.GetValue() == "ugly");
  
  prop.Apply(GetSTC());
  REQUIRE( prop.is_ok());
  
  prop.ApplyReset(GetSTC());
  REQUIRE( prop.is_ok());
  
  pugi::xml_document doc;
  REQUIRE( doc.load_string("<property name = \"fold.comment\">2</property>"));

  wex::property prop2(doc.document_element());
  REQUIRE( prop2.GetName() == "fold.comment");
  REQUIRE( prop2.GetValue() == "2");
  REQUIRE( prop2.is_ok());
}
