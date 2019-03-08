////////////////////////////////////////////////////////////////////////////////
// Name:      test-file.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <chrono>
#include <wex/file.h>
#include "../test.h"

TEST_CASE( "wex::file" ) 
{
  SUBCASE( "basic" ) 
  {
    REQUIRE(!wex::file("XXXXX").is_opened());
    
    wex::file file(wex::test::get_path("test.h"));
  
    REQUIRE(!file.check_sync());
    REQUIRE(!file.get_contents_changed());
    REQUIRE( file.is_opened());
    
    file.reset_contents_changed();

    REQUIRE(!file.file_save());
    REQUIRE( file.file_save("test-save"));
    REQUIRE( file.get_filename().stat().is_ok());
    // The fullpath should be normalized, test it.
    REQUIRE( file.get_filename().data().string() != "./test.h");
    REQUIRE(!file.get_filename().stat().is_readonly());
    REQUIRE( file.file_load(wex::test::get_path("test.bin")));
    REQUIRE( file.open(wex::test::get_path("test.bin").data().string()));
    REQUIRE( file.is_opened());

    const std::string* buffer = file.read();
    REQUIRE(buffer->length() == 40);
    
    REQUIRE( file.file_new("test-xxx"));
    REQUIRE( file.open(std::ios_base::out));
    REQUIRE( file.is_opened());
    REQUIRE( file.write(std::string("OK")));
    REQUIRE( file.write(*buffer));

    wex::file create(std::string("test-create"), std::ios_base::out);
    REQUIRE( create.is_opened());
    REQUIRE( create.write(std::string("OK")));
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
    wex::file file(wex::test::get_path("test.h"));
  
    const int max = 10000;
    const auto ex_start = std::chrono::system_clock::now();
    
    for (int i = 0; i < max; i++)
    {
      REQUIRE(file.read()->length() > 0);
    }

    const auto ex_milli = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - ex_start);
    
    CHECK(ex_milli.count() < 2000);
  }
}
