////////////////////////////////////////////////////////////////////////////////
// Name:      test-filename.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <chrono>
#include "../catch.hpp"
#include "../test.h"

TEST_CASE( "wxExFileName" ) 
{
  wxExFileName fileName(GetTestFile());
  
  SECTION( "basic" ) 
  {
    REQUIRE(!wxExLexers::Get()->GetLexers().empty());
    INFO(fileName.GetLexer().GetScintillaLexer());
    REQUIRE(fileName.GetLexer().GetScintillaLexer() == "cpp");
    REQUIRE(fileName.GetStat().IsOk());
    fileName.Assign("xxx");
    REQUIRE(fileName.GetStat().IsOk());
    fileName.SetLexer(wxExLexer("ada"));
    REQUIRE(fileName.GetLexer().GetScintillaLexer() == "ada");
  }

  SECTION( "timing" ) 
  {
    const int max = 1000;
    const wxExFileName exfile(GetTestFile());
    const auto ex_start = std::chrono::system_clock::now();

    for (int i = 0; i < max; i++)
    {
      REQUIRE(!exfile.GetStat().IsReadOnly());
    }

    const auto ex_milli = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - ex_start);
    const wxFileName file(GetTestFile());
    const auto wx_start = std::chrono::system_clock::now();

    for (int j = 0; j < max; j++)
    {
      REQUIRE(file.IsFileWritable());
    }

    const auto wx_milli = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - wx_start);

    CHECK(ex_milli.count() < 1000);
    CHECK(wx_milli.count() < 1000);

    INFO(wxString::Format(
      "wxExFileName::IsReadOnly %d files in %d ms wxFileName::IsFileWritable %d files in %d ms",
      max,
      (int)ex_milli.count(),
      max,
      (int)wx_milli.count()).ToStdString());
  }
}
