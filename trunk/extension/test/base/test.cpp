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

void wxExTestFixture::setUp()
{
  m_Config = new wxExConfig("test.cfg", wxCONFIG_USE_LOCAL_FILE);
  m_File = new wxExFile("test.h");
  m_FileName = new wxExFileName("test.h");
  m_FileNameStatistics = new wxExFileNameStatistics("test.h");
  m_Lexer = new wxExLexer();
  m_Lexers = new wxExLexers(wxExFileName("../../data/lexers.xml"));
  m_RCS = new wxExRCS();
  m_Stat = new wxExStat("test.h");
  m_Statistics = new wxExStatistics<long>();
  m_Tool = new wxExTool(ID_TOOL_REPORT_COUNT);
}

void wxExTestFixture::testConstructors()
{
  CPPUNIT_ASSERT(m_File != NULL);
}

void wxExTestFixture::testMethods()
{
  // test wxExConfig
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
  m_Config->Set("Author", "myauthor");
  CPPUNIT_ASSERT(m_Config->Get("Author", "yourauthor") == "myauthor");

  // test wxExFile
  CPPUNIT_ASSERT(m_File->GetStat().IsOk());
  CPPUNIT_ASSERT(m_File->GetStat().GetFullPath() == m_File->GetFileName().GetFullPath());
  // The fullpath should be normalized, test it.
  CPPUNIT_ASSERT(m_File->GetFileName().GetFullPath() != "test.h");
  CPPUNIT_ASSERT(!m_File->GetStat().IsReadOnly());
  CPPUNIT_ASSERT(!m_File->CheckSyncNeeded());
  CPPUNIT_ASSERT(!m_File->GetStat().IsReadOnly());
  CPPUNIT_ASSERT(m_File->FileOpen(wxExFileName("test.bin")));
  wxCharBuffer buffer = m_File->Read();
  CPPUNIT_ASSERT(buffer.length() == 40);

  // test wxExFileName
  CPPUNIT_ASSERT(m_FileName->GetLexer().GetScintillaLexer().empty());
  CPPUNIT_ASSERT(m_FileName->GetStat().IsOk());
  m_FileName->Assign("xxx");
  m_FileName->GetStat().Update("xxx");
  CPPUNIT_ASSERT(!m_FileName->GetStat().IsOk());

  // test wxExFileNameStatistics
  CPPUNIT_ASSERT(m_FileNameStatistics->Get().empty());
  CPPUNIT_ASSERT(m_FileNameStatistics->Get("xx") == 0);

  // test wxExLexer
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

  // test wxExLexers
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
  CPPUNIT_ASSERT(m_Lexer->GetCommentEnd2().empty());
  CPPUNIT_ASSERT(!m_Lexer->GetKeywords().empty());
  CPPUNIT_ASSERT(!m_Lexer->GetKeywordsSet().empty());
  CPPUNIT_ASSERT(!m_Lexer->GetKeywordsString().empty());
  CPPUNIT_ASSERT(!m_Lexer->GetProperties().empty());
  CPPUNIT_ASSERT(m_Lexer->IsKeyword("class"));
  CPPUNIT_ASSERT(m_Lexer->IsKeyword("const"));
  CPPUNIT_ASSERT(m_Lexer->KeywordStartsWith("cla"));
  CPPUNIT_ASSERT(!m_Lexer->KeywordStartsWith("xxx"));
  CPPUNIT_ASSERT(!m_Lexer->MakeComment("test", true).empty());
  CPPUNIT_ASSERT(!m_Lexer->MakeComment("test", "test").empty());
  CPPUNIT_ASSERT(m_Lexer->SetKeywords("hello:1"));

  // test wxExRCS
  CPPUNIT_ASSERT(m_RCS->GetDescription().empty());
  CPPUNIT_ASSERT(m_RCS->GetUser().empty());

  // test wxExStat
  CPPUNIT_ASSERT(!m_Stat->IsLink());
  CPPUNIT_ASSERT(m_Stat->IsOk());
  CPPUNIT_ASSERT(!m_Stat->IsReadOnly());
  CPPUNIT_ASSERT(m_Stat->Update("testlink"));
//  CPPUNIT_ASSERT(m_Stat->IsLink());

  // test wxExStatistics
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

  // test wxExTextFile
  m_TextFile = new wxExTextFile(wxExFileName("test.h"), m_Config, m_Lexers);
  CPPUNIT_ASSERT(m_TextFile->RunTool(ID_TOOL_REPORT_COUNT));
  CPPUNIT_ASSERT(!m_TextFile->GetStatistics().GetElements().GetItems().empty());
  CPPUNIT_ASSERT(!m_TextFile->IsOpened()); // file should be closed after running tool

  CPPUNIT_ASSERT(m_TextFile->RunTool(ID_TOOL_REPORT_COUNT)); // do the same test
  CPPUNIT_ASSERT(!m_TextFile->GetStatistics().GetElements().GetItems().empty());
  CPPUNIT_ASSERT(!m_TextFile->IsOpened()); // file should be closed after running tool

  CPPUNIT_ASSERT(m_TextFile->RunTool(ID_TOOL_REPORT_KEYWORD));
//  CPPUNIT_ASSERT(!m_TextFile->GetStatistics().GetKeywords().GetItems().empty());

  // test wxExTool
  CPPUNIT_ASSERT(m_Tool->IsStatisticsType() > 0);

  // test various wxExMethods
  const wxString header = wxExHeader(m_TextFile->GetFileName(), m_Config, "test");
  CPPUNIT_ASSERT(header.Contains("test"));
}

void wxExTestFixture::testTiming()
{
  wxExFile file("test.h");

  CPPUNIT_ASSERT(file.IsOpened());

  wxStopWatch sw;

  const int max = 10000;

  for (int i = 0; i < max; i++)
  {
    wxCharBuffer buffer = file.Read();
    CPPUNIT_ASSERT(buffer.length() > 0);
  }

  const long exfile_read = sw.Time();

  sw.Start();

  wxFile wxfile("test.h");

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

  printf(
    "wxExFile::Read:%ld wxFile::Read:%ld\n",
    exfile_read,
    file_read);
}

void wxExTestFixture::testTimingAttrib()
{
  const int max = 1000;

  wxStopWatch sw;

  const wxExFileName exfile("test.h");

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
    "wxExFileName::IsReadOnly:%ld wxFileName::IsFileWritable:%ld\n",
    exfile_time,
    file_time);
}

void wxExTestFixture::testTimingConfig()
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
    "wxExConfig::Get:%ld wxConfig::Read:%ld\n",
    exconfig,
    config);
}

void wxExTestFixture::tearDown()
{
}

wxExTestSuite::wxExTestSuite()
  : CppUnit::TestSuite("wxextension test suite")
{
  // Add the tests.
  addTest(new CppUnit::TestCaller<wxExTestFixture>(
    "testConstructors",
    &wxExTestFixture::testConstructors));

  addTest(new CppUnit::TestCaller<wxExTestFixture>(
    "testMethods",
    &wxExTestFixture::testMethods));

  addTest(new CppUnit::TestCaller<wxExTestFixture>(
    "testTiming",
    &wxExTestFixture::testTiming));

  addTest(new CppUnit::TestCaller<wxExTestFixture>(
    "testTimingAttrib",
    &wxExTestFixture::testTimingAttrib));

  addTest(new CppUnit::TestCaller<wxExTestFixture>(
    "testTimingConfig",
    &wxExTestFixture::testTimingConfig));
}
