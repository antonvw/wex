////////////////////////////////////////////////////////////////////////////////
// Name:      test-file.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <chrono>
#include <wex/file.h>
#include "../test.h"

TEST_CASE( "wex::file" ) 
{
  SUBCASE( "basic" ) 
  {
    REQUIRE(!wex::file("XXXXX").IsOpened());
    
    wex::file file(GetTestPath("test.h"));
  
    REQUIRE(!file.CheckSync());
    REQUIRE(!file.GetContentsChanged());
    REQUIRE( file.IsOpened());
    
    file.ResetContentsChanged();

    REQUIRE(!file.FileSave());
    REQUIRE( file.FileSave("test-save"));
    REQUIRE( file.GetFileName().GetStat().is_ok());
    // The fullpath should be normalized, test it.
    REQUIRE( file.GetFileName().Path().string() != "./test.h");
    REQUIRE(!file.GetFileName().GetStat().is_readonly());
    REQUIRE( file.FileLoad(GetTestPath("test.bin")));
    REQUIRE( file.Open(GetTestPath("test.bin").Path().string()));
    REQUIRE( file.IsOpened());

    const std::string* buffer = file.Read();
    REQUIRE(buffer->length() == 40);
    
    REQUIRE( file.FileNew("test-xxx"));
    REQUIRE( file.Open(std::ios_base::out));
    REQUIRE( file.IsOpened());
    REQUIRE( file.Write(std::string("OK")));
    REQUIRE( file.Write(*buffer));

    wex::file create(std::string("test-create"), std::ios_base::out);
    REQUIRE( create.IsOpened());
    REQUIRE( create.Write(std::string("OK")));
  }

  // file should be closed before remove (at least for windows)
  SUBCASE( "remove")
  {
    REQUIRE( remove("test-create") == 0);
    REQUIRE( remove("test-save") == 0);
    REQUIRE( remove("test-xxx") == 0);
  }

  SUBCASE( "timing" ) 
  {
    wex::file file(GetTestPath("test.h"));
  
    const int max = 10000;
    const auto ex_start = std::chrono::system_clock::now();
    
    for (int i = 0; i < max; i++)
    {
      REQUIRE(file.Read()->length() > 0);
    }

    const auto ex_milli = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - ex_start);
    
    CHECK(ex_milli.count() < 2000);
  }
}
