////////////////////////////////////////////////////////////////////////////////
// Name:      test-dir.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/extension/dir.h>
#include "../test.h"

TEST_CASE( "wxExDir" ) 
{
  SUBCASE( "Not recursive" ) 
  {
    wxExDir dir(GetTestDir(), "*.h", DIR_FILES);
    REQUIRE(dir.DirExists());
    REQUIRE(dir.GetFlags() == DIR_FILES);
    REQUIRE(dir.GetFileSpec() == "*.h");
    REQUIRE(dir.FindFiles() == 2);
  }
  
  SUBCASE( "Recursive" ) 
  {
    // we could use *.h;*.cpp, however wxDir handles only
    // one type, so all files would be found (wxExDir uses empty spec,
    // and checks each file on a match)
    wxExDir dir("../../", "*.h", DIR_FILES | DIR_DIRS);
    REQUIRE(dir.DirExists());
    REQUIRE(dir.GetFileSpec() == "*.h");
    REQUIRE(dir.FindFiles() > 50);
  }

  SUBCASE( "Invalid" ) 
  {
    wxExDir dir("xxxx", "*.h", DIR_FILES);
    REQUIRE(!dir.DirExists());
  }
}
