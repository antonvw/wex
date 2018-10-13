////////////////////////////////////////////////////////////////////////////////
// Name:      test-path.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <chrono>
#include <wx/extension/path.h>
#include "../test.h"

TEST_CASE( "wex::path" ) 
{
  SUBCASE( "Constructor" ) 
  {
    REQUIRE( wex::path().Path().empty());
    REQUIRE( wex::path("xxx").Path().string() == "xxx");
    REQUIRE( wex::path(wex::path("yyy")).Path().string() == "yyy");
    wex::path fn = GetTestPath("test.h");
    REQUIRE( fn.GetLexer().GetScintillaLexer() == "cpp");
    REQUIRE( wex::path(fn).GetFullName() == "test.h");
    REQUIRE( wex::path("..").IsRelative());
    REQUIRE(!wex::path("..").IsAbsolute());
    REQUIRE( wex::path("xx") == wex::path("xx"));
    REQUIRE( wex::path("xx") != wex::path("xy"));
    REQUIRE(!wex::path().GetOriginal().empty());
    REQUIRE(!wex::path().Current().empty());
  }
  
  SUBCASE( "Basic" ) 
  {
    wex::path path(GetTestPath("test.h"));
  
    REQUIRE(!path.DirExists());
    REQUIRE( path.FileExists());
    REQUIRE( path.GetExtension() == ".h");
    REQUIRE( path.GetFullName() == "test.h");
    REQUIRE(!path.Path().empty());
    REQUIRE( path.GetLexer().GetScintillaLexer() == "cpp");
    REQUIRE( path.GetName() == "test");
    REQUIRE(!path.GetPath().empty());
    REQUIRE(!path.GetPaths().empty());
    REQUIRE( path.GetStat().IsOk());
    REQUIRE(!path.IsReadOnly());

    REQUIRE( path.Append("error").Path().string().find("error") != std::string::npos);
    REQUIRE(!path.Canonical("xxx"));

    path.ReplaceFileName("xxx");

    REQUIRE(!wex::path("XXXXX").GetStat().IsOk());
    REQUIRE(!wex::path("XXXXX").OpenMIME());

    REQUIRE( wex::path("XXXXX").MakeAbsolute().GetFullName() == "XXXXX");
    REQUIRE( wex::path("XXXXX").MakeAbsolute().Path().string() != "XXXXX");
    REQUIRE( wex::path("XXXXX").MakeAbsolute("yyy").GetFullName() == "XXXXX");
  }

  SUBCASE( "Timing" ) 
  {
    const int max = 1000;
    const wex::path exfile(GetTestPath("test.h"));
    const auto ex_start = std::chrono::system_clock::now();

    for (int i = 0; i < max; i++)
    {
      REQUIRE(!exfile.GetStat().IsReadOnly());
    }

    const auto ex_milli = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - ex_start);
    const wex::path file(GetTestPath("test.h"));
    const auto wx_start = std::chrono::system_clock::now();

    for (int j = 0; j < max; j++)
    {
      REQUIRE(!file.IsReadOnly());
    }

    const auto wx_milli = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - wx_start);

    CHECK(ex_milli.count() < 1000);
    CHECK(wx_milli.count() < 1000);
  }
}
