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
#include <wx/xml/xml.h>
#include <wx/extension/indicator.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/stc.h>
#include "test.h"

TEST_CASE("wxExIndicator")
{
  wxExIndicator ind;
  REQUIRE(!ind.IsOk() );
  
  wxExIndicator indx(5, 2);
  wxExIndicator indy(7, 5);
  
  REQUIRE( indx.IsOk());
  REQUIRE( indy.IsOk());
  REQUIRE( indx < indy );
  
  wxXmlNode xml(wxXML_ELEMENT_NODE, "indicator");
  xml.AddAttribute("no", "5");
  xml.SetContent("symbol,green");

  wxExIndicator ind2(&xml);
  REQUIRE( ind2.GetNo() == 5);
  REQUIRE( ind2.IsOk());
  
  wxExSTC* stc = new wxExSTC(GetFrame(), "hello stc");
  ind2.Apply(stc);
  REQUIRE( ind2.IsOk());
}
