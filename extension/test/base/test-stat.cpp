////////////////////////////////////////////////////////////////////////////////
// Name:      test-stat.cpp
// Purpose:   Implementation for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/extension/stat.h>
#include "test.h"

void TestFixture::testStat()
{
  wxExStat stat(GetTestFile().GetFullPath());

  CPPUNIT_ASSERT( stat.IsOk());
  CPPUNIT_ASSERT(!stat.IsReadOnly());
  CPPUNIT_ASSERT( stat.Sync("./test-base.link"));
  CPPUNIT_ASSERT(!stat.GetModificationTime().empty());
}
