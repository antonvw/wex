////////////////////////////////////////////////////////////////////////////////
// Name:      test-path.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <chrono>
#include <wex/path.h>
#include "../test.h"

TEST_CASE( "wex::path" ) 
{
  SUBCASE( "Constructor" ) 
  {
    REQUIRE( wex::path().data().empty());
    REQUIRE( wex::path("xxx").data().string() == "xxx");
    REQUIRE( wex::path(wex::path("yyy")).data().string() == "yyy");
    wex::path fn = wex::test::get_path("test.h");
    REQUIRE( fn.lexer().scintilla_lexer() == "cpp");
    REQUIRE( wex::path(fn).fullname() == "test.h");
    REQUIRE( wex::path("..").is_relative());
    REQUIRE(!wex::path("..").is_absolute());
    REQUIRE( wex::path("xx") == wex::path("xx"));
    REQUIRE( wex::path("xx") != wex::path("xy"));
    REQUIRE(!wex::path().original().empty());
    REQUIRE(!wex::path().current().empty());
  }
  
  SUBCASE( "Basic" ) 
  {
    wex::path path(wex::test::get_path("test.h"));
  
    REQUIRE(!path.dir_exists());
    REQUIRE( path.file_exists());
    REQUIRE( path.extension() == ".h");
    REQUIRE( path.fullname() == "test.h");
    REQUIRE(!path.data().empty());
    REQUIRE( path.lexer().scintilla_lexer() == "cpp");
    REQUIRE( path.name() == "test");
    REQUIRE(!path.get_path().empty());
    REQUIRE(!path.paths().empty());
    REQUIRE( path.stat().is_ok());
    REQUIRE(!path.is_readonly());

    REQUIRE( path.append("error").data().string().find("error") != std::string::npos);

    path.replace_filename("xxx");

    REQUIRE(!wex::path("XXXXX").stat().is_ok());
    REQUIRE(!wex::path("XXXXX").open_mime());

    REQUIRE( wex::path("XXXXX").make_absolute().fullname() == "XXXXX");
    REQUIRE( wex::path("XXXXX").make_absolute().data().string() != "XXXXX");
  }

  SUBCASE( "Timing" ) 
  {
    const int max = 1000;
    const wex::path exfile(wex::test::get_path("test.h"));
    const auto ex_start = std::chrono::system_clock::now();

    for (int i = 0; i < max; i++)
    {
      REQUIRE(!exfile.stat().is_readonly());
    }

    const auto ex_milli = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - ex_start);
    const wex::path file(wex::test::get_path("test.h"));
    const auto wx_start = std::chrono::system_clock::now();

    for (int j = 0; j < max; j++)
    {
      REQUIRE(!file.is_readonly());
    }

    const auto wx_milli = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - wx_start);

    CHECK(ex_milli.count() < 1000);
    CHECK(wx_milli.count() < 1000);
  }
}
