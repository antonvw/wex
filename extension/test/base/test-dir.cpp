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
    REQUIRE(dir.GetDir().DirExists());
    REQUIRE(dir.GetFlags() == DIR_FILES);
    REQUIRE(dir.GetFileSpec() == "*.h");
    REQUIRE(dir.FindFiles() == 2);
  }
  
  SUBCASE( "Recursive" ) 
  {
    // we could use *.h;*.cpp, however wxDir handles only
    // one type, so all files would be found (wxExDir uses empty spec,
    // and checks each file on a match)
    wxExDir dir("../../", "*.h");
    REQUIRE(dir.GetDir().DirExists());
    REQUIRE(dir.GetFileSpec() == "*.h");
    REQUIRE(dir.FindFiles() > 50);
  }

  SUBCASE( "Invalid" ) 
  {
    wxExDir dir("xxxx", "*.h", DIR_FILES);
    REQUIRE(!dir.GetDir().DirExists());
  }

  SUBCASE( "GetAllFiles" ) 
  {
    REQUIRE(wxExGetAllFiles(std::string("./"), "*.txt", DIR_FILES).size() == 4);
    REQUIRE(wxExGetAllFiles(wxExPath("./"), "*.txt", DIR_DIRS).empty());
  }
}
