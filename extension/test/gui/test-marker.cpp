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
  wxExMarker marker;
  REQUIRE( !marker.IsOk() );
  
  wxExMarker markerx(5, 2);
  wxExMarker markery(7, 5);
  
  REQUIRE( markerx.IsOk());
  REQUIRE( markery.IsOk());
  REQUIRE( markerx < markery );
  
  wxXmlNode xml(wxXML_ELEMENT_NODE, "marker");
  xml.AddAttribute("no", "5");
  xml.SetContent("symbol,green,white");

  wxExMarker marker2(&xml);
  REQUIRE( marker2.GetNo() == 5);
  REQUIRE( marker2.IsOk());
  
  markerx.Apply(GetSTC());
}
