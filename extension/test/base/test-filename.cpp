////////////////////////////////////////////////////////////////////////////////
// Name:      test-filename.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/stopwatch.h>
#include "../catch.hpp"
#include "../test.h"

TEST_CASE( "wxExFileName" ) 
{
  wxExFileName fileName(GetTestFile());
  
  SECTION( "basic" ) 
  {
    REQUIRE(fileName.GetLexer().GetScintillaLexer().empty());
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

    REQUIRE(exfile_time < 10);
    REQUIRE(file_time < 100);
    
    INFO(wxString::Format(
      "wxExFileName::IsReadOnly %d files in %ld ms wxFileName::IsFileWritable %d files in %ld ms",
      max,
      exfile_time,
      max,
      file_time).ToStdString());
  }
}
