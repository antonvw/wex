////////////////////////////////////////////////////////////////////////////////
// Name:      test.cpp
// Purpose:   Implementation for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2012 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/config.h>
#include "test.h"

#define TEST_FILE "./test.h"
#define TEST_BIN "./test.bin"

void TestFixture::testConfig()
{
  wxConfig* cfg = new wxConfig(
    wxEmptyString, 
    wxEmptyString, 
    "test.cfg", 
    wxEmptyString, 
    wxCONFIG_USE_LOCAL_FILE);
    
  const int max = 100000;

  wxStopWatch sw;
  sw.Start();

  for (int j = 0; j < max; j++)
  {
    cfg->Read("test", 0l);
  }

  const long config = sw.Time();

  Report(wxString::Format(
    "reading %d items from config took %ld milliseconds", max, config));
}

void TestFixture::testDir()
{
  wxExDir dir("./", "*.h", wxDIR_FILES);
  
  CPPUNIT_ASSERT(dir.IsOpened());
  CPPUNIT_ASSERT(dir.GetFileSpec() == "*.h");
  CPPUNIT_ASSERT(dir.FindFiles() == 1);
}
  
void TestFixture::testFile()
{
  wxExFile file(wxExFileName(TEST_FILE));
  
  CPPUNIT_ASSERT(!file.CheckSync());

  CPPUNIT_ASSERT(!file.GetContentsChanged());

  CPPUNIT_ASSERT( file.GetFileName().GetStat().IsOk());
  // The fullpath should be normalized, test it.
  CPPUNIT_ASSERT( file.GetFileName().GetFullPath() != TEST_FILE);
  CPPUNIT_ASSERT(!file.GetFileName().GetStat().IsReadOnly());

  CPPUNIT_ASSERT( file.FileLoad(wxExFileName(TEST_BIN)));
  CPPUNIT_ASSERT(!file.IsOpened());
  
  CPPUNIT_ASSERT( file.Open(wxExFileName(TEST_BIN).GetFullPath()));
  CPPUNIT_ASSERT( file.IsOpened());

  wxCharBuffer buffer = file.Read();
  CPPUNIT_ASSERT(buffer.length() == 40);
}
  
void TestFixture::testFileName()
{
  wxExFileName fileName(TEST_FILE);
  
  CPPUNIT_ASSERT(fileName.GetLexer().GetScintillaLexer().empty());
  CPPUNIT_ASSERT(fileName.GetStat().IsOk());
  fileName.Assign("xxx");
  CPPUNIT_ASSERT(fileName.GetStat().IsOk());
}

void TestFixture::testFileStatistics()
{
  wxExFileStatistics fileStatistics;
  wxExTextFile textFile(wxExFileName(TEST_FILE), ID_TOOL_REPORT_COUNT);
  
  CPPUNIT_ASSERT(fileStatistics.Get().empty());
  CPPUNIT_ASSERT(fileStatistics.Get("xx") == 0);

  CPPUNIT_ASSERT( textFile.RunTool());
  CPPUNIT_ASSERT(!textFile.GetStatistics().GetElements().GetItems().empty());

  fileStatistics += textFile.GetStatistics();
  
  CPPUNIT_ASSERT(!fileStatistics.Get().empty());
}

void TestFixture::testStat()
{
  wxExStat stat(TEST_FILE);

  CPPUNIT_ASSERT( stat.IsOk());
  CPPUNIT_ASSERT(!stat.IsReadOnly());
  CPPUNIT_ASSERT( stat.Sync("./test-base.link"));
}

void TestFixture::testStatistics()
{
  wxExStatistics<long> statistics;
  statistics.Inc("test");
  CPPUNIT_ASSERT(statistics.Get("test") == 1);
  statistics.Inc("test");
  CPPUNIT_ASSERT(statistics.Get("test") == 2);
  statistics.Set("test", 13);
  CPPUNIT_ASSERT(statistics.Get("test") == 13);
  statistics.Dec("test");
  CPPUNIT_ASSERT(statistics.Get("test") == 12);
  statistics.Inc("test2");
  CPPUNIT_ASSERT(statistics.Get("test2") == 1);
  CPPUNIT_ASSERT(statistics.Get().Contains("test"));
  CPPUNIT_ASSERT(statistics.Get().Contains("test2"));

  wxExStatistics<long> copy(statistics);
  CPPUNIT_ASSERT(copy.Get("test") == 12);
  CPPUNIT_ASSERT(copy.Get("test2") == 1);

  statistics.Clear();
  CPPUNIT_ASSERT(statistics.GetItems().empty());
  CPPUNIT_ASSERT(!copy.GetItems().empty());
}

void TestFixture::testTiming()
{
  wxExFile file(wxExFileName(TEST_FILE));

  CPPUNIT_ASSERT(file.IsOpened());

  wxStopWatch sw;

  const int max = 10000;

  for (int i = 0; i < max; i++)
  {
    CPPUNIT_ASSERT(file.Read().length() > 0);
  }

  const long exfile_read = sw.Time();

  sw.Start();

  wxFile wxfile(TEST_FILE);

  for (int j = 0; j < max; j++)
  {
    char* charbuffer = new char[wxfile.Length()];
    wxfile.Read(charbuffer, wxfile.Length());
    wxString* buffer = new wxString(charbuffer, wxfile.Length());
    CPPUNIT_ASSERT(buffer->length() > 0);
    delete charbuffer;
    delete buffer;
  }

  const long file_read = sw.Time();

  Report(wxString::Format(
    "wxExFile::Read:%ld wxFile::Read:%ld",
    exfile_read,
    file_read));
}

void TestFixture::testTimingAttrib()
{
  const int max = 1000;

  wxStopWatch sw;

  const wxExFileName exfile(TEST_FILE);

  int checked = 0;

  for (int i = 0; i < max; i++)
  {
    checked += exfile.GetStat().IsReadOnly();
  }

  const long exfile_time = sw.Time();

  sw.Start();

  const wxFileName file(TEST_FILE);

  for (int j = 0; j < max; j++)
  {
    checked += file.IsFileWritable();
  }

  const long file_time = sw.Time();

  Report(wxString::Format(
    "wxExFileName::IsReadOnly:%ld wxFileName::IsFileWritable:%ld",
    exfile_time,
    file_time));
}

void TestFixture::testTool()
{
  CPPUNIT_ASSERT(wxExTool(ID_TOOL_REPORT_COUNT).IsCount());
  CPPUNIT_ASSERT(wxExTool(ID_TOOL_REPORT_FIND).IsFindType());
  CPPUNIT_ASSERT(wxExTool(ID_TOOL_REPORT_REPLACE).IsFindType());
  CPPUNIT_ASSERT(wxExTool(ID_TOOL_REPORT_COUNT).IsStatisticsType());
  CPPUNIT_ASSERT(wxExTool(ID_TOOL_REPORT_COUNT).IsReportType());
}

wxExTestSuite::wxExTestSuite()
  : CppUnit::TestSuite("wxExtension test suite")
{
  addTest(new CppUnit::TestCaller<TestFixture>(
    "testConfig",
    &TestFixture::testConfig));
    
  addTest(new CppUnit::TestCaller<TestFixture>(
    "testDir",
    &TestFixture::testDir));

  addTest(new CppUnit::TestCaller<TestFixture>(
    "testFile",
    &TestFixture::testFile));

  addTest(new CppUnit::TestCaller<TestFixture>(
    "testFileName",
    &TestFixture::testFileName));

  addTest(new CppUnit::TestCaller<TestFixture>(
    "testFileStatistics",
    &TestFixture::testFileStatistics));

  addTest(new CppUnit::TestCaller<TestFixture>(
    "testStat",
    &TestFixture::testStat));

  addTest(new CppUnit::TestCaller<TestFixture>(
    "testStatistics",
    &TestFixture::testStatistics));

  addTest(new CppUnit::TestCaller<TestFixture>(
    "testTiming",
    &TestFixture::testTiming));

  addTest(new CppUnit::TestCaller<TestFixture>(
    "testTimingAttrib",
    &TestFixture::testTimingAttrib));
    
  addTest(new CppUnit::TestCaller<TestFixture>(
    "testTool",
    &TestFixture::testTool));
}
