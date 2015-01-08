////////////////////////////////////////////////////////////////////////////////
// Name:      test-statistics.cpp
// Purpose:   Implementation for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/extension/textfile.h>
#include "test.h"

void TestFixture::testStatistics()
{
  wxExStatistics<long> statistics;
  statistics.Inc("test");
  CPPUNIT_ASSERT(statistics.Get("test") == 1);
  statistics.Inc("test");
  CPPUNIT_ASSERT(statistics.Get("test") == 2);
  statistics.Set("test", 13);
  CPPUNIT_ASSERT(statistics.Get("test") == 13);
  statistics.Dec("test");
  CPPUNIT_ASSERT(statistics.Get("test") == 12);
  statistics.Inc("test2");
  CPPUNIT_ASSERT(statistics.Get("test2") == 1);
  CPPUNIT_ASSERT(statistics.Get().Contains("test"));
  CPPUNIT_ASSERT(statistics.Get().Contains("test2"));

  wxExStatistics<long> copy(statistics);
  CPPUNIT_ASSERT(copy.Get("test") == 12);
  CPPUNIT_ASSERT(copy.Get("test2") == 1);

  statistics.Clear();
  CPPUNIT_ASSERT(statistics.GetItems().empty());
  CPPUNIT_ASSERT(!copy.GetItems().empty());
}
