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
    wxExPath path(GetTestFile());
  
    REQUIRE(!path.DirExists());
    REQUIRE( path.FileExists());
    REQUIRE( path.GetExtension() == ".h");
    REQUIRE( path.GetFullName() == "test.h");
    REQUIRE(!path.GetFullPath().empty());
    REQUIRE( path.GetLexer().GetScintillaLexer() == "cpp");
    REQUIRE( path.GetName() == "test");
    REQUIRE(!path.GetPath().empty());
    REQUIRE(!path.GetPaths().empty());
    REQUIRE( path.GetStat().IsOk());
    REQUIRE(!path.IsReadOnly());
    path.GetVolume();

    REQUIRE( path.Append("error").GetFullPath().find("error") != std::string::npos);
    REQUIRE(!path.Canonical("xxx"));

    path.ReplaceFileName("xxx");

    REQUIRE(!wxExPath("XXXXX").GetStat().IsOk());

    REQUIRE( wxExPath("XXXXX").MakeAbsolute().GetFullName() == "XXXXX");
    REQUIRE( wxExPath("XXXXX").MakeAbsolute().GetFullPath() != "XXXXX");
    REQUIRE( wxExPath("XXXXX").MakeAbsolute("yyy").GetFullName() == "XXXXX");
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
