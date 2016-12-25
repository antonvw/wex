////////////////////////////////////////////////////////////////////////////////
// Name:      test-property.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/property.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/stc.h>
#include "test.h"

TEST_CASE("wxExProperty", "[stc][lexer]")
{
  wxExProperty inv;
  REQUIRE( !inv.IsOk() );
  
  wxExProperty prop("man", "ugly");
  
  REQUIRE( prop.IsOk());
  REQUIRE( prop.GetName() == "man");
  REQUIRE( prop.GetValue() == "ugly");
  
  prop.Apply(GetSTC());
  REQUIRE( prop.IsOk());
  
  prop.ApplyReset(GetSTC());
  REQUIRE( prop.IsOk());
  
  pugi::xml_document doc;
  REQUIRE( doc.load_string("<property name = \"fold.comment\">2</property>"));

  wxExProperty prop2(doc.document_element());
  REQUIRE( prop2.GetName() == "fold.comment");
  REQUIRE( prop2.GetValue() == "2");
  REQUIRE( prop2.IsOk());
}
