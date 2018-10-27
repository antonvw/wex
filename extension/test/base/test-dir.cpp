////////////////////////////////////////////////////////////////////////////////
// Name:      test-dir.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/dir.h>
#include "../test.h"

TEST_CASE( "wex::dir" ) 
{
  SUBCASE( "Not recursive" ) 
  {
    wex::dir dir(GetTestPath(), "*.h", wex::dir::FILES);
    REQUIRE(dir.GetDir().DirExists());
    REQUIRE(dir.GetFlags() == wex::dir::FILES);
    REQUIRE(dir.GetFileSpec() == "*.h");
    REQUIRE(dir.FindFiles() == 2);
  }
  
  SUBCASE( "Recursive" ) 
  {
    wex::dir dir("../../", "*.h");
    REQUIRE(dir.GetDir().DirExists());
    REQUIRE(dir.GetFileSpec() == "*.h");
    REQUIRE(dir.FindFiles() > 50);
  }

  SUBCASE( "Invalid" ) 
  {
    wex::dir dir("xxxx", "*.h", wex::dir::FILES);
    REQUIRE(!dir.GetDir().DirExists());
  }

  SUBCASE( "GetAllFiles" ) 
  {
    REQUIRE(wex::get_all_files(std::string("./"), "*.txt", wex::dir::FILES).size() == 4);
    REQUIRE(wex::get_all_files(wex::path("./"), "*.txt", wex::dir::DIRS).empty());
  }
}
