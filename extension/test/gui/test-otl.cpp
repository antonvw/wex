////////////////////////////////////////////////////////////////////////////////
// Name:      test-otl.cpp
// Purpose:   Implementation for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/otl.h>
#include "test.h"

void wxExGuiTestFixture::testOTL()
{
#if wxExUSE_OTL
  wxExOTL otl;
  
  CPPUNIT_ASSERT( otl.Datasource().empty());
  CPPUNIT_ASSERT(!otl.IsConnected());
  CPPUNIT_ASSERT(!otl.Logoff());
  CPPUNIT_ASSERT( otl.Query("select * from world") == 0);
#endif
}
