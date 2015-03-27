////////////////////////////////////////////////////////////////////////////////
// Name:      test-marker.cpp
// Purpose:   Implementation for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/xml/xml.h>
#include <wx/extension/marker.h>
#include "test.h"

void fixture::testMarker()
{
  wxExMarker marker;
  CPPUNIT_ASSERT( !marker.IsOk() );
  
  wxExMarker markerx(5, 2);
  wxExMarker markery(7, 5);
  
  CPPUNIT_ASSERT( markerx.IsOk());
  CPPUNIT_ASSERT( markery.IsOk());
  CPPUNIT_ASSERT( markerx < markery );
  
  wxXmlNode xml(wxXML_ELEMENT_NODE, "marker");
  xml.AddAttribute("no", "5");
  xml.SetContent("symbol,green,white");

  wxExMarker marker2(&xml);
  CPPUNIT_ASSERT( marker2.GetNo() == 5);
  CPPUNIT_ASSERT( marker2.IsOk());
}
