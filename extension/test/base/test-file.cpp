////////////////////////////////////////////////////////////////////////////////
// Name:      test-file.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/stopwatch.h>
#include <wx/extension/file.h>
#include "../catch.hpp"
#include "../test.h"

TEST_CASE( "wxExFile" ) 
{
  wxExFile file(GetTestFile());
  
  SECTION( "basic" ) 
  {
    REQUIRE(!file.CheckSync());

    REQUIRE(!file.GetContentsChanged());
    
    file.ResetContentsChanged();

    REQUIRE(!file.FileSave());

    REQUIRE( file.GetFileName().GetStat().IsOk());
    // The fullpath should be normalized, test it.
    REQUIRE( file.GetFileName().GetFullPath() != GetTestFile().GetFullPath());
    REQUIRE(!file.GetFileName().GetStat().IsReadOnly());

    REQUIRE(!file.FileLoad(wxExFileName(GetTestDir() + "test.bin")));
    REQUIRE(!file.IsOpened());
    
    REQUIRE( file.Open(wxExFileName(GetTestDir() + "test.bin").GetFullPath()));
    REQUIRE( file.IsOpened());

    wxCharBuffer buffer = file.Read();
    REQUIRE(buffer.length() == 40);
    
    file.FileNew(wxExFileName("xxx"));
  }

  SECTION( "timing" ) 
  {
    REQUIRE(file.IsOpened());
    REQUIRE(file.Length() > 0);

    wxStopWatch sw;

    const int max = 10000;

    for (int i = 0; i < max; i++)
    {
      REQUIRE(file.Read().length() > 0);
    }

    const long exfile_read = sw.Time();

    wxFile wxfile(GetTestFile().GetFullPath());
    REQUIRE(wxfile.IsOpened());
    const size_t l = wxfile.Length();
    REQUIRE(l > 0);
    
    sw.Start();

    for (int j = 0; j < max; j++)
    {
      char* charbuffer = new char[l];
      wxfile.Read(charbuffer, l);
      REQUIRE(sizeof(charbuffer) > 0);
      delete[] charbuffer;
    }

    const long file_read = sw.Time();

    CHECK(exfile_read < 2000);
    CHECK(file_read < 2000);
  }
}
