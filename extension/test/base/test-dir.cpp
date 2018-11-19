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
    wex::dir dir(GetTestPath(), "*.h", wex::dir::type_t().set(wex::dir::FILES));
    REQUIRE(dir.get_path().dir_exists());
    REQUIRE(dir.type().test(wex::dir::FILES));
    REQUIRE(dir.file_spec() == "*.h");
    REQUIRE(dir.find_files() == 2);
  }
  
  SUBCASE( "Recursive" ) 
  {
    wex::dir dir("../../", "*.h");
    REQUIRE(dir.get_path().dir_exists());
    REQUIRE(dir.file_spec() == "*.h");
    REQUIRE(dir.find_files() > 50);
  }

  SUBCASE( "Invalid" ) 
  {
    wex::dir dir("xxxx", "*.h", wex::dir::type_t().set(wex::dir::FILES));
    REQUIRE(!dir.get_path().dir_exists());
  }

  SUBCASE( "GetAllFiles" ) 
  {
    REQUIRE(wex::get_all_files(std::string("./"), "*.txt", 
      wex::dir::type_t().set(wex::dir::FILES)).size() == 4);
    REQUIRE(wex::get_all_files(wex::path("./"), "*.txt", 
      wex::dir::type_t().set(wex::dir::DIRS)).empty());
  }
}
