////////////////////////////////////////////////////////////////////////////////
// Name:      test-file.cpp
// Purpose:   Implementation for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/stopwatch.h>
#include <wx/extension/file.h>
#include "test.h"

void TestFixture::testFile()
{
  wxExFile file(GetTestFile());
  
  CPPUNIT_ASSERT(!file.CheckSync());

  CPPUNIT_ASSERT(!file.GetContentsChanged());

  CPPUNIT_ASSERT( file.GetFileName().GetStat().IsOk());
  // The fullpath should be normalized, test it.
  CPPUNIT_ASSERT( file.GetFileName().GetFullPath() != GetTestFile().GetFullPath());
  CPPUNIT_ASSERT(!file.GetFileName().GetStat().IsReadOnly());

  CPPUNIT_ASSERT(!file.FileLoad(wxExFileName(GetTestDir() + "test.bin")));
  CPPUNIT_ASSERT(!file.IsOpened());
  
  CPPUNIT_ASSERT( file.Open(wxExFileName(GetTestDir() + "test.bin").GetFullPath()));
  CPPUNIT_ASSERT( file.IsOpened());

  wxCharBuffer buffer = file.Read();
  CPPUNIT_ASSERT(buffer.length() == 40);
  
  file.FileNew(wxExFileName("xxx"));
}

void TestFixture::testFileTiming()
{
  wxExFile file(GetTestFile());
  CPPUNIT_ASSERT(file.IsOpened());
  CPPUNIT_ASSERT(file.Length() > 0);

  wxStopWatch sw;

  const int max = 10000;

  for (int i = 0; i < max; i++)
  {
    CPPUNIT_ASSERT(file.Read().length() > 0);
  }

  const long exfile_read = sw.Time();

  wxFile wxfile(GetTestFile().GetFullPath());
  CPPUNIT_ASSERT(wxfile.IsOpened());
  const size_t l = wxfile.Length();
  CPPUNIT_ASSERT(l > 0);
  
  sw.Start();

  for (int j = 0; j < max; j++)
  {
    char* charbuffer = new char[l];
    wxfile.Read(charbuffer, l);
    CPPUNIT_ASSERT(sizeof(charbuffer) > 0);
    delete[] charbuffer;
  }

  const long file_read = sw.Time();

  CPPUNIT_ASSERT(exfile_read < 200);
  CPPUNIT_ASSERT(file_read < 200);
  
  Report(wxString::Format(
    "wxExFile::Read %d items in %ld ms wxFile::Read %d items in %ld ms",
    max,
    exfile_read,
    max,
    file_read).ToStdString());
}
