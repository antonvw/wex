////////////////////////////////////////////////////////////////////////////////
// Name:      test-filename.cpp
// Purpose:   Implementation for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/stopwatch.h>
#include "test.h"

void TestFixture::testFileName()
{
  wxExFileName fileName(GetTestFile());
  
  CPPUNIT_ASSERT(fileName.GetLexer().GetScintillaLexer().empty());
  CPPUNIT_ASSERT(fileName.GetStat().IsOk());
  fileName.Assign("xxx");
  CPPUNIT_ASSERT(fileName.GetStat().IsOk());
}

void TestFixture::testFileNameTiming()
{
  const int max = 1000;

  wxStopWatch sw;

  const wxExFileName exfile(GetTestFile());

  for (int i = 0; i < max; i++)
  {
    CPPUNIT_ASSERT(!exfile.GetStat().IsReadOnly());
  }

  const long exfile_time = sw.Time();

  sw.Start();

  const wxFileName file(GetTestFile());

  for (int j = 0; j < max; j++)
  {
    CPPUNIT_ASSERT(file.IsFileWritable());
  }

  const long file_time = sw.Time();

  CPPUNIT_ASSERT(exfile_time < 10);
  CPPUNIT_ASSERT(file_time < 10);
  
  Report(wxString::Format(
    "wxExFileName::IsReadOnly %d files in %ld ms wxFileName::IsFileWritable %d files in %ld ms",
    max,
    exfile_time,
    max,
    file_time).ToStdString());
}
