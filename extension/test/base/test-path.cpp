////////////////////////////////////////////////////////////////////////////////
// Name:      test-path.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <chrono>
#include <wx/extension/path.h>
#include "../test.h"

TEST_CASE( "wxExPath" ) 
{
  SUBCASE( "Constructor" ) 
  {
    REQUIRE( wxExPath().GetFullPath().empty());
    REQUIRE( wxExPath("xxx").GetFullPath() == "xxx");
    REQUIRE( wxExPath(wxExPath("yyy")).GetFullPath() == "yyy");
    wxExPath fn = wxExPath(GetTestFile());
    REQUIRE( fn.GetLexer().GetScintillaLexer() == "cpp");
    REQUIRE( wxExPath(fn).GetFullName() == "test.h");
    REQUIRE( wxExPath("..").IsRelative());
    REQUIRE(!wxExPath("..").IsAbsolute());
    REQUIRE( wxExPath("xx") == wxExPath("xx"));
    REQUIRE( wxExPath("xx") != wxExPath("xy"));
  }
  
  SUBCASE( "Basic" ) 
  {
    wxExPath fileName(GetTestFile());
  
    REQUIRE( fileName.DirExists());
    REQUIRE( fileName.FileExists());
    REQUIRE( fileName.GetExtension() == "h");
    REQUIRE( fileName.GetFullName() == "test.h");
    REQUIRE(!fileName.GetFullPath().empty());
    REQUIRE( fileName.GetLexer().GetScintillaLexer() == "cpp");
    REQUIRE( fileName.GetName() == "test");
    REQUIRE(!fileName.GetPath().empty());
    REQUIRE( fileName.GetStat().IsOk());
    REQUIRE(!fileName.IsReadOnly());

    REQUIRE(!wxExPath("XXXXX").GetStat().IsOk());
    REQUIRE( wxExPath("XXXXX").MakeAbsolute());
  }

  SUBCASE( "Timing" ) 
  {
    const int max = 1000;
    const wxExPath exfile(GetTestFile());
    const auto ex_start = std::chrono::system_clock::now();

    for (int i = 0; i < max; i++)
    {
      REQUIRE(!exfile.GetStat().IsReadOnly());
    }

    const auto ex_milli = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - ex_start);
    const wxExPath file(GetTestFile());
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
