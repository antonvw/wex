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
#include <wx/extension/marker.h>
#include "test.h"

void wxExGuiTestFixture::testMarker()
{
  wxExMarker marker;
  CPPUNIT_ASSERT( !marker.IsOk() );
  
  wxExMarker markerx(5, 2);
  wxExMarker markery(7, 5);
  
  CPPUNIT_ASSERT( markerx.IsOk());
  CPPUNIT_ASSERT( markery.IsOk());
  CPPUNIT_ASSERT( markerx < markery );
}
