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
#include <wx/xml/xml.h>
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
  
  wxExSTC* stc = new wxExSTC(GetFrame(), "hello stc");
  AddPane(GetFrame(), stc);
  
  prop.Apply(stc);
  REQUIRE( prop.IsOk());
  
  prop.ApplyReset(stc);
  REQUIRE( prop.IsOk());
  
  wxXmlNode xml(wxXML_ELEMENT_NODE, "property");
  xml.AddAttribute("name", "fold.comment");
  new wxXmlNode(&xml, wxXML_TEXT_NODE , "","2");

  wxExProperty prop2(&xml);
  REQUIRE( prop2.GetName() == "fold.comment");
  REQUIRE( prop2.GetValue() == "2");
  REQUIRE( prop2.IsOk());
}
