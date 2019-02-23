////////////////////////////////////////////////////////////////////////////////
// Name:      test-dir.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/dir.h>
#include "../test.h"

TEST_CASE( "wex::dir" ) 
{
  SUBCASE( "Not recursive" ) 
  {
    wex::dir dir(get_testpath(), 
      "*.h", std::string(), wex::dir::type_t().set(wex::dir::FILES));
    REQUIRE(dir.get_path().dir_exists());
    REQUIRE(dir.type().test(wex::dir::FILES));
    REQUIRE(dir.file_spec() == "*.h");
    REQUIRE(dir.dir_spec().empty());
    REQUIRE(dir.find_files() == 2);
  }
  
  SUBCASE( "Recursive" ) 
  {
    wex::dir dir("../../", "*.h");
    REQUIRE(dir.get_path().dir_exists());
    REQUIRE(dir.file_spec() == "*.h");
    REQUIRE(dir.find_files() > 50);
  }

  SUBCASE( "Match folders" ) 
  {
    wex::log::verbose() << wex::path::current();
    wex::dir dir("../../", std::string(), "data", 
      wex::dir::type_t().set(wex::dir::DIRS));
    REQUIRE(dir.get_path().dir_exists());
    REQUIRE(dir.file_spec().empty());
    REQUIRE(dir.dir_spec() == "data");
    REQUIRE(dir.find_files() == 1);
  }

  SUBCASE( "Invalid" ) 
  {
    wex::dir dir(
      "xxxx", "*.h", std::string(), wex::dir::type_t().set(wex::dir::FILES));
    REQUIRE(!dir.get_path().dir_exists());
  }

  SUBCASE( "get_all_files" ) 
  {
    REQUIRE(wex::get_all_files(
      std::string("./"), "*.txt", std::string(),
      wex::dir::type_t().set(wex::dir::FILES)).size() == 4);
    REQUIRE(wex::get_all_files(wex::path("./"), "*.txt", std::string(),
      wex::dir::type_t().set(wex::dir::DIRS)).empty());
  }
}
