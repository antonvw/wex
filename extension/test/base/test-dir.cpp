////////////////////////////////////////////////////////////////////////////////
// Name:      test-dir.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/extension/dir.h>
#include "../test.h"

TEST_CASE( "wxExDir" ) 
{
  wxExDir dir(GetTestDir(), "*.h", wxDIR_FILES);
  REQUIRE(dir.IsOpened());
  REQUIRE(dir.GetFileSpec() == "*.h");
  REQUIRE(dir.FindFiles() == 2);
  
  // we could use *.h;*.cpp, however wxDir handles only
  // one type, so all files would be found (wxExDir uses empty spec,
  // and checks each file on a match)
  wxExDir dir2("../../", "*.h", wxDIR_FILES | wxDIR_DIRS);
  REQUIRE(dir2.IsOpened());
  REQUIRE(dir2.GetFileSpec() == "*.h");
  REQUIRE(dir2.FindFiles() > 50);
}
