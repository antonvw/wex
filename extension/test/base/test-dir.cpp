////////////////////////////////////////////////////////////////////////////////
// Name:      test-dir.cpp
// Purpose:   Implementation for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/extension/dir.h>
#include "test.h"

void TestFixture::testDir()
{
  wxExDir dir(GetTestDir(), "*.h", wxDIR_FILES);
  CPPUNIT_ASSERT(dir.IsOpened());
  CPPUNIT_ASSERT(dir.GetFileSpec() == "*.h");
  CPPUNIT_ASSERT(dir.FindFiles() == 2);
  
  // we could use *.h;*.cpp, however wxDir handles only
  // one type, so all files would be found (wxExDir uses empty spec,
  // and checks each file on a match)
  wxExDir dir2("../../", "*.h", wxDIR_FILES | wxDIR_DIRS);
  CPPUNIT_ASSERT(dir2.IsOpened());
  CPPUNIT_ASSERT(dir2.GetFileSpec() == "*.h");
  CPPUNIT_ASSERT(dir2.FindFiles() > 50);
}
