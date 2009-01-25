/******************************************************************************\
* File:          test.cpp
* Purpose:       Implementation for wxextension cpp unit testing
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
* Created:       za 17 jan 2009 11:51:20 CET
*
* Copyright (c) 2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <TestCaller.h>
#include "test.h"

void exTestFixture::setUp()
{
  m_File = new exFile("test.h");
  m_FileName = new exFileName("test.h");
  m_FileNameStatistics = new exFileNameStatistics("test.h");
  m_Lexer = new exLexer();
  m_RCS = new exRCS();
  m_Stat = new exStat("test.h");
  m_Statistics = new exStatistics<long>();
  m_Tool = new exTool(ID_TOOL_REPORT_COUNT);
}

void exTestFixture::testConstructors()
{
  CPPUNIT_ASSERT(m_File != NULL);
}

void exTestFixture::testMethods()
{
  // test exFile
  CPPUNIT_ASSERT(m_File->GetStat().IsOk());
  CPPUNIT_ASSERT(m_File->GetFileName().GetFullPath() == "test.h");

  // test exFileName
  CPPUNIT_ASSERT(m_FileName->GetStat().IsOk());

  // test exLexer
  m_Lexer->SetLexerFromText("// this is a cpp comment text");
  CPPUNIT_ASSERT(m_Lexer->GetScintillaLexer().empty()); // we have no lexers

  // test exRCS
  CPPUNIT_ASSERT(m_RCS->GetUser().empty());

  // test exStat
  CPPUNIT_ASSERT(m_Stat->IsOk());

  // test exStatistics
  m_Statistics->Inc("test");
  CPPUNIT_ASSERT(m_Statistics->Get("test") == 1);
  m_Statistics->Inc("test");
  CPPUNIT_ASSERT(m_Statistics->Get("test") == 2);
  m_Statistics->Set("test", 13);
  CPPUNIT_ASSERT(m_Statistics->Get("test") == 13);
  m_Statistics->Dec("test");
  CPPUNIT_ASSERT(m_Statistics->Get("test") == 12);
  m_Statistics->Inc("test2");
  CPPUNIT_ASSERT(m_Statistics->Get("test2") == 1);

  // test exTool
  CPPUNIT_ASSERT(m_Tool->IsStatisticsType() > 0);
}

void exTestFixture::testTiming()
{
  exFile file("test.h");

  CPPUNIT_ASSERT(file.IsOpened());
  
  wxStopWatch sw;

  const int max = 10000;

  for (int i = 0; i < max; i++)
  {
    wxString* buffer = file.Read();
    CPPUNIT_ASSERT(buffer !° NULL);
    delete buffer;
  }

  const long exfile_read = sw.Time();

  sw.Start();

  wxFile wxfile("test.h");

  for (int j = 0; j < max; j++)
  {
    char* charbuffer = new char[wxfile.Length()];
    wxfile.Read(charbuffer, wxfile.Length());
    wxString* buffer = new wxString(charbuffer, wxfile.Length());
    delete charbuffer;
    delete buffer;
  }

  const long file_read = sw.Time();

  printf(
    "exFile::Read:%ld wxFile::Read:%ld\n",
    exfile_read,
    file_read);
}

void exTestFixture::testTimingAttrib()
{
  const int max = 1000;

  wxStopWatch sw;

  const exFileName exfile("test.h");

  int checked = 0;

  for (int i = 0; i < max; i++)
  {
    checked += exfile.GetStat().IsReadOnly();
  }

  const long exfile_time = sw.Time();

  sw.Start();

  const wxFileName file("test.h");

  for (int j = 0; j < max; j++)
  {
    checked += file.IsFileWritable();
  }

  const long file_time = sw.Time();

  printf(
    "exFileName::IsReadOnly:%ld wxFileName::IsFileWritable:%ld\n",
    exfile_time,
    file_time);
}

void exTestFixture::tearDown()
{
}

void exAppTestFixture::setUp()
{
}

void exAppTestFixture::testConstructors()
{
  m_App = new exApp();
  m_Dir = new exDir("./");
  m_SVN = new exSVN(SVN_STAT, "test.h");
}

void exAppTestFixture::testMethods()
{
  // test exApp
  m_App->OnInit();

  CPPUNIT_ASSERT(m_App->GetConfig() != NULL);
  CPPUNIT_ASSERT(m_App->GetLexers() != NULL);
  CPPUNIT_ASSERT(m_App->GetPrinter() != NULL);

  // test exDir
  CPPUNIT_ASSERT(m_Dir->GetFiles().GetCount() > 0);
  
  // test exSVN
  m_SVN->Get();
}

void exAppTestFixture::testTimingConfig()
{
}

void exAppTestFixture::tearDown()
{
  const int max = 100000;

  wxStopWatch sw;

  for (int i = 0; i < max; i++)
  {
    exApp::GetConfig("test", 0);
  }

  const long exconfig = sw.Time();

  sw.Start();

  for (int j = 0; j < max; j++)
  {
    exApp::GetConfig()->Read("test", 0l);
  }

  const long config = sw.Time();

  printf(
    "exConfig::Get:%ld wxConfig::Read:%ld\n",
    exconfig,
    config);
  }
}

exTestSuite::exTestSuite()
  : CppUnit::TestSuite("wxextension test suite")

{
  // Add the tests.
  addTest(new CppUnit::TestCaller<exTestFixture>(
    "testConstructors",
    &exTestFixture::testConstructors));

  addTest(new CppUnit::TestCaller<exTestFixture>(
    "testMethods",
    &exTestFixture::testMethods));

  addTest(new CppUnit::TestCaller<exTestFixture>(
    "testTiming",
    &exTestFixture::testTiming));

  addTest(new CppUnit::TestCaller<exTestFixture>(
    "testTimingAttrib",
    &exTestFixture::testTimingAttrib));

/* TODO: the exApp or wxApp not yet okay without normal wxApp initialization

  addTest(new CppUnit::TestCaller<exAppTestFixture>(
    "testConstructors",
    &exAppTestFixture::testConstructors));

  addTest(new CppUnit::TestCaller<exAppTestFixture>(
    "testMethods",
    &exAppTestFixture::testMethods));
    
  addTest(new CppUnit::TestCaller<exAppTestFixture>(
    "testTimingConfig",
    &exAppTestFixture::testTimingConfig));
    */
}
