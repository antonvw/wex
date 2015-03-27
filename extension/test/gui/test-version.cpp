////////////////////////////////////////////////////////////////////////////////
// Name:      test-version.cpp
// Purpose:   Implementation for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/version.h>
#include "test.h"

void fixture::testVersion()
{
  CPPUNIT_ASSERT(!wxExVersionInfo().GetVersionOnlyString().empty());
  CPPUNIT_ASSERT(!wxExGetVersionInfo().GetVersionOnlyString().empty());
}
