////////////////////////////////////////////////////////////////////////////////
// Name:      test-file.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <chrono>
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
    REQUIRE( file.IsOpened());
    
    file.ResetContentsChanged();

    REQUIRE(!file.FileSave());
    REQUIRE( file.GetFileName().GetStat().IsOk());
    // The fullpath should be normalized, test it.
    REQUIRE( file.GetFileName().GetFullPath() != GetTestFile().GetFullPath());
    REQUIRE(!file.GetFileName().GetStat().IsReadOnly());
    REQUIRE(!file.FileLoad(wxExFileName(GetTestDir() + "test.bin")));
    REQUIRE( file.Open(wxExFileName(GetTestDir() + "test.bin").GetFullPath()));
    REQUIRE( file.IsOpened());

    const wxCharBuffer* buffer = file.Read();
    REQUIRE(buffer->length() == 40);
    
    file.FileNew(wxExFileName("test-xxx"));
    
    REQUIRE( file.Open(wxFile::write));
    REQUIRE( file.Write(*buffer));
    REQUIRE( file.Write(wxString("OK")));
    
    wxExFile create(wxExFileName("test-create"), wxFile::write);
    REQUIRE( create.IsOpened());
    REQUIRE( create.Write(wxString("OK")));
    (void)remove("test-create");
  }

  SECTION( "timing" ) 
  {
    const int max = 10000;
    const auto ex_start = std::chrono::system_clock::now();
    
    for (int i = 0; i < max; i++)
    {
      REQUIRE(file.Read()->length() > 0);
    }

    const auto ex_milli = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - ex_start);
    
    wxFile wxfile(GetTestFile().GetFullPath());
    const size_t l = wxfile.Length();
    const auto wx_start = std::chrono::system_clock::now();
    
    for (int j = 0; j < max; j++)
    {
      char* charbuffer = new char[l];
      wxfile.Read(charbuffer, l);
      REQUIRE(sizeof(charbuffer) > 0);
      delete[] charbuffer;
    }

    const auto wx_milli = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - wx_start);

    CHECK(ex_milli.count() < 2000);
    CHECK(wx_milli.count() < 2000);
  }
}
