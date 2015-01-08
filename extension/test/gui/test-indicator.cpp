////////////////////////////////////////////////////////////////////////////////
// Name:      test-indicator.cpp
// Purpose:   Implementation for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/indicator.h>
#include "test.h"

void wxExGuiTestFixture::testIndicator()
{
  wxExIndicator ind;
  CPPUNIT_ASSERT(!ind.IsOk() );
  
  wxExIndicator indx(5, 2);
  wxExIndicator indy(7, 5);
  
  CPPUNIT_ASSERT( indx.IsOk());
  CPPUNIT_ASSERT( indy.IsOk());
  CPPUNIT_ASSERT( indx < indy );
}
