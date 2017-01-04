////////////////////////////////////////////////////////////////////////////////
// Name:      test-filename.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <chrono>
#include <wx/extension/filename.h>
#include "../test.h"

TEST_CASE( "wxExFileName" ) 
{
  wxExFileName fileName(GetTestFile());
  
  SUBCASE( "basic" ) 
  {
    REQUIRE( fileName.DirExists());
    REQUIRE( fileName.FileExists());
    REQUIRE( fileName.GetExtension() == "h");
    REQUIRE( fileName.GetFullName() == "test.h");
    REQUIRE(!fileName.GetFullPath().empty());
    REQUIRE( fileName.GetLexer().GetScintillaLexer() == "cpp");
    REQUIRE( fileName.GetName() == "test");
    REQUIRE(!fileName.GetPath().empty());
    REQUIRE( fileName.GetStat().IsOk());
    REQUIRE( fileName.IsOk());
    REQUIRE(!fileName.IsReadOnly());

    REQUIRE(!wxExFileName("XXXXX").GetStat().IsOk());
    REQUIRE( wxExFileName("XXXXX").MakeAbsolute());
    
    fileName.SetLexer(wxExLexer("ada"));
    REQUIRE(fileName.GetLexer().GetScintillaLexer() == "ada");
  }

  SUBCASE( "timing" ) 
  {
    const int max = 1000;
    const wxExFileName exfile(GetTestFile());
    const auto ex_start = std::chrono::system_clock::now();

    for (int i = 0; i < max; i++)
    {
      REQUIRE(!exfile.GetStat().IsReadOnly());
    }

    const auto ex_milli = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - ex_start);
    const wxExFileName file(GetTestFile());
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
