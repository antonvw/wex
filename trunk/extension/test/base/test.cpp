/******************************************************************************\
* File:          test.cpp
* Purpose:       Implementation for wxextension cpp unit testing
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id: test.cpp 450 2009-03-11 20:02:41Z antonvw $
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
  m_Config = new exConfig("test.cfg", wxCONFIG_USE_LOCAL_FILE);
  m_File = new exFile("test.h");
  m_FileName = new exFileName("test.h");
  m_FileNameStatistics = new exFileNameStatistics("test.h");
  m_Lexer = new exLexer();
  m_Lexers = new exLexers(exFileName("../../data/lexers.xml"));
  m_RCS = new exRCS();
  m_Stat = new exStat("test.h");
  m_Statistics = new exStatistics<long>();
  m_TextFile = new exTextFile(exFileName("test.h"), m_Config, m_Lexers);
  m_Tool = new exTool(ID_TOOL_REPORT_COUNT);
}

void exTestFixture::testConstructors()
{
  CPPUNIT_ASSERT(m_File != NULL);
}

void exTestFixture::testMethods()
{
  // test exConfig
  CPPUNIT_ASSERT(m_Config->Get("keystring", "val") == "val");
  CPPUNIT_ASSERT(m_Config->Get("keylong", 12) == 12);
  CPPUNIT_ASSERT(m_Config->GetBool("keybool", true));
  CPPUNIT_ASSERT(m_Config->GetFindReplaceData() != NULL);
  m_Config->Set("keystring", "val2");
  CPPUNIT_ASSERT(m_Config->Get("keystring", "val") == "val2");
  m_Config->Set("keylong", 15);
  CPPUNIT_ASSERT(m_Config->Get("keylong", 7) == 15);
  m_Config->SetBool("keybool", false);
  CPPUNIT_ASSERT(m_Config->GetBool("keybool", true) == false);
  m_Config->Toggle("keybool");
  CPPUNIT_ASSERT(m_Config->GetBool("keybool", false));

  // test exFile
  CPPUNIT_ASSERT(m_File->GetStat().IsOk());
  CPPUNIT_ASSERT(m_File->GetStat().GetFullPath() == m_File->GetFileName().GetFullPath());
  // The fullpath should be normalized, test it.
  CPPUNIT_ASSERT(m_File->GetFileName().GetFullPath() != "test.h");
  CPPUNIT_ASSERT(!m_File->GetStat().IsReadOnly());
  CPPUNIT_ASSERT(!m_File->CheckSyncNeeded());
  CPPUNIT_ASSERT(!m_File->GetStat().IsReadOnly());
  CPPUNIT_ASSERT(m_File->FileOpen(exFileName("test.bin")));
  wxString* buffer = m_File->Read();
  CPPUNIT_ASSERT(buffer != NULL);
  CPPUNIT_ASSERT(buffer->size() == 40);
  delete buffer;

  // test exFileName
  CPPUNIT_ASSERT(m_FileName->GetLexer().GetScintillaLexer().empty());
  CPPUNIT_ASSERT(m_FileName->GetStat().IsOk());
  m_FileName->Assign("xxx");
  m_FileName->GetStat().Update("xxx");
  CPPUNIT_ASSERT(!m_FileName->GetStat().IsOk());

  // test exFileNameStatistics
  CPPUNIT_ASSERT(m_FileNameStatistics->Get().empty());
  CPPUNIT_ASSERT(m_FileNameStatistics->Get("xx") == 0);

  // test exLexer
  *m_Lexer = m_Lexers->FindByText("// this is a cpp comment text");
  CPPUNIT_ASSERT(m_Lexer->GetScintillaLexer().empty()); // we have no lexers
  m_Lexer->SetKeywords("test11 test21:1 test31:1 test12:2 test22:2");
  CPPUNIT_ASSERT(m_Lexer->IsKeyword("test11"));
  CPPUNIT_ASSERT(m_Lexer->IsKeyword("test21"));
  CPPUNIT_ASSERT(m_Lexer->IsKeyword("test12"));
  CPPUNIT_ASSERT(m_Lexer->IsKeyword("test22"));
  CPPUNIT_ASSERT(m_Lexer->KeywordStartsWith("te"));
  CPPUNIT_ASSERT(!m_Lexer->KeywordStartsWith("xx"));
  CPPUNIT_ASSERT(!m_Lexer->GetKeywords().empty());
  CPPUNIT_ASSERT(!m_Lexer->GetKeywordsSet().empty());

  // test exLexers
  CPPUNIT_ASSERT(m_Lexers->Read());
  *m_Lexer = m_Lexers->FindByText("// this is a cpp comment text");
  CPPUNIT_ASSERT(m_Lexer->GetScintillaLexer() == "cpp");
  CPPUNIT_ASSERT(m_Lexers->FindByFileName(wxFileName("test.h")).GetScintillaLexer() == "cpp");
  CPPUNIT_ASSERT(m_Lexers->FindByName("cpp").GetScintillaLexer() == "cpp");
  CPPUNIT_ASSERT(m_Lexers->Count() > 0);
  CPPUNIT_ASSERT(!m_Lexers->BuildWildCards(wxFileName("test.h")).empty());
  CPPUNIT_ASSERT(!m_Lexers->GetIndicators().empty());
  CPPUNIT_ASSERT(!m_Lexers->GetMarkers().empty());
  CPPUNIT_ASSERT(!m_Lexers->GetStyles().empty());
  CPPUNIT_ASSERT(!m_Lexers->GetStylesHex().empty());
  CPPUNIT_ASSERT(!m_Lexer->GetAssociations().empty());
  CPPUNIT_ASSERT(!m_Lexer->GetColourings().empty());
  CPPUNIT_ASSERT(!m_Lexer->GetCommentBegin().empty());
  CPPUNIT_ASSERT(!m_Lexer->GetCommentBegin2().empty());
  CPPUNIT_ASSERT(!m_Lexer->GetCommentEnd().empty());
  CPPUNIT_ASSERT(!m_Lexer->GetCommentEnd2().empty());
  CPPUNIT_ASSERT(!m_Lexer->GetKeywords().empty());
  CPPUNIT_ASSERT(!m_Lexer->GetKeywordsSet().empty());
  CPPUNIT_ASSERT(!m_Lexer->GetKeywordsString().empty());
  CPPUNIT_ASSERT(!m_Lexer->GetProperties().empty());
  CPPUNIT_ASSERT(m_Lexer->GetScintillaLexer() != "cpp");
  CPPUNIT_ASSERT(m_Lexer->IsKeyword("class"));
  CPPUNIT_ASSERT(m_Lexer->IsKeyword("const"));
  CPPUNIT_ASSERT(m_Lexer->KeywordStartsWith("cla"));
  CPPUNIT_ASSERT(!m_Lexer->KeywordStartsWith("xxx"));
  CPPUNIT_ASSERT(!m_Lexer->MakeComment("test", true).empty());
  CPPUNIT_ASSERT(!m_Lexer->MakeComment("test", "test").empty());
  CPPUNIT_ASSERT(m_Lexer->SetKeywords("hello:1"));

  // test exRCS
  CPPUNIT_ASSERT(m_RCS->GetDescription().empty());
  CPPUNIT_ASSERT(m_RCS->GetUser().empty());

  // test exStat
  CPPUNIT_ASSERT(!m_Stat->IsLink());
  CPPUNIT_ASSERT(m_Stat->IsOk());
  CPPUNIT_ASSERT(!m_Stat->IsReadOnly());
  CPPUNIT_ASSERT(m_Stat->Update("testlink"));
//  CPPUNIT_ASSERT(m_Stat->IsLink());

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
  m_Statistics->Clear();
  CPPUNIT_ASSERT(m_Statistics->GetItems().empty());

  // test exTextFile
  CPPUNIT_ASSERT(m_TextFile->RunTool(ID_TOOL_REPORT_COUNT));
  CPPUNIT_ASSERT(!m_TextFile->GetStatistics().GetElements().GetItems().empty());
  CPPUNIT_ASSERT(!m_TextFile->IsOpened()); // file should be closed after running tool

  CPPUNIT_ASSERT(m_TextFile->RunTool(ID_TOOL_REPORT_COUNT)); // do the same test
  CPPUNIT_ASSERT(!m_TextFile->GetStatistics().GetElements().GetItems().empty());
  CPPUNIT_ASSERT(!m_TextFile->IsOpened()); // file should be closed after running tool

  CPPUNIT_ASSERT(m_TextFile->RunTool(ID_TOOL_REPORT_KEYWORD));
//  CPPUNIT_ASSERT(!m_TextFile->GetStatistics().GetKeywords().GetItems().empty());

  // test exTool
  CPPUNIT_ASSERT(m_Tool->IsStatisticsType() > 0);

  // test various exMethods
  const wxString header = exHeader(m_TextFile->GetFileName(), m_Config, "test");
  CPPUNIT_ASSERT(header.Contains("test"));
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
    CPPUNIT_ASSERT(buffer != NULL);
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

void exTestFixture::testTimingConfig()
{
  const int max = 100000;

  wxStopWatch sw;

  for (int i = 0; i < max; i++)
  {
    m_Config->Get("test", 0);
  }

  const long exconfig = sw.Time();

  sw.Start();

  for (int j = 0; j < max; j++)
  {
    m_Config->Read("test", 0l);
  }

  const long config = sw.Time();

  printf(
    "exConfig::Get:%ld wxConfig::Read:%ld\n",
    exconfig,
    config);
}

void exTestFixture::tearDown()
{
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

  addTest(new CppUnit::TestCaller<exTestFixture>(
    "testTimingConfig",
    &exTestFixture::testTimingConfig));
}
