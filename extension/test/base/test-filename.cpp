////////////////////////////////////////////////////////////////////////////////
// Name:      test-filename.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/stopwatch.h>
#include "../catch.hpp"
#include "../test.h"

TEST_CASE( "wxExFileName" ) 
{
  wxExFileName fileName(GetTestFile());
  
  SECTION( "basic" ) 
  {
    REQUIRE(wxExLexers::Get()->GetCount() > 0);
    INFO(fileName.GetLexer().GetScintillaLexer());
    REQUIRE(fileName.GetLexer().GetScintillaLexer() == "cpp");
    REQUIRE(fileName.GetStat().IsOk());
    fileName.Assign("xxx");
    REQUIRE(fileName.GetStat().IsOk());
  }

  SECTION( "timing" ) 
  {
    const int max = 1000;

    wxStopWatch sw;

    const wxExFileName exfile(GetTestFile());

    for (int i = 0; i < max; i++)
    {
      REQUIRE(!exfile.GetStat().IsReadOnly());
    }

    const long exfile_time = sw.Time();

    sw.Start();

    const wxFileName file(GetTestFile());

    for (int j = 0; j < max; j++)
    {
      REQUIRE(file.IsFileWritable());
    }

    const long file_time = sw.Time();

    CHECK(exfile_time < 1000);
    CHECK(file_time < 1000);
    
    INFO(wxString::Format(
      "wxExFileName::IsReadOnly %d files in %ld ms wxFileName::IsFileWritable %d files in %ld ms",
      max,
      exfile_time,
      max,
      file_time).ToStdString());
  }
}
