/******************************************************************************\
* File:          test.cpp
* Purpose:       Implementation for wxExtension cpp unit testing
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
* Created:       za 17 jan 2009 11:51:20 CET
*
* Copyright (c) 2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <TestCaller.h>
#include <wx/extension/extension.h>
#include "test.h"

#define TEST_FILE "./test.h"
#define TEST_BIN "./test.bin"

class wxExFileTest: public wxExFile
{
public :
  wxExFileTest(const wxString& filename) : wxExFile(filename) {;};
private:
  virtual bool GetContentsChanged() const {return false;};
  virtual void ResetContentsChanged() {;};
  virtual void DoFileLoad(bool) {;};
  virtual void DoFileSave(bool) {;};
};

void wxExTestFixture::testFile()
{
  wxExFileTest file(TEST_FILE);
  
  CPPUNIT_ASSERT(file.GetStat().IsOk());
  CPPUNIT_ASSERT(file.GetStat().GetFullPath() == file.GetFileName().GetFullPath());
  // The fullpath should be normalized, test it.
  CPPUNIT_ASSERT(file.GetFileName().GetFullPath() != TEST_FILE);
  CPPUNIT_ASSERT(!file.GetStat().IsReadOnly());
  file.CheckFileSync();
  CPPUNIT_ASSERT(!file.GetStat().IsReadOnly());
  CPPUNIT_ASSERT(file.FileLoad(TEST_BIN));
  CPPUNIT_ASSERT(!file.IsOpened());
  CPPUNIT_ASSERT(file.Open(wxExFileName(TEST_BIN).GetFullPath()));
  wxCharBuffer buffer = file.Read();
  CPPUNIT_ASSERT(buffer.length() == 40);
}
  
void wxExTestFixture::testFileName()
{
  wxExFileName fileName(TEST_FILE);
  
  CPPUNIT_ASSERT(!fileName.GetLexer().GetScintillaLexer().empty());
  CPPUNIT_ASSERT(fileName.GetStat().IsOk());
  fileName.Assign("xxx");
  CPPUNIT_ASSERT(fileName.GetStat().IsOk());
}

void wxExTestFixture::testFileStatistics()
{
  wxExFileStatistics fileStatistics();
  
  CPPUNIT_ASSERT(fileStatistics.Get().empty());
  CPPUNIT_ASSERT(fileStatistics.Get("xx") == 0);
}

void wxExTestFixture::testLexer()
{
  wxExLexers* m_Lexers;
  m_Lexers = new wxExLexers(wxFileName("../extension/data/lexers.xml"));
  
  wxExLexer* m_Lexer;
  m_Lexer = new wxExLexer();
  
  *m_Lexer = m_Lexers->FindByText("// this is a cpp comment text");
  CPPUNIT_ASSERT(m_Lexer->GetScintillaLexer().empty());
  // now read lexers
  m_Lexers->Read();
  *m_Lexer = m_Lexers->FindByText("// this is a cpp comment text");
  CPPUNIT_ASSERT(!m_Lexer->GetExtensions().empty());
  CPPUNIT_ASSERT(!m_Lexer->GetCommentBegin().empty());
  CPPUNIT_ASSERT(!m_Lexer->GetCommentBegin2().empty());
  CPPUNIT_ASSERT(m_Lexer->GetCommentEnd().empty());
  CPPUNIT_ASSERT(!m_Lexer->GetCommentEnd2().empty());
  CPPUNIT_ASSERT(!m_Lexer->GetKeywords().empty());
  CPPUNIT_ASSERT(!m_Lexer->GetKeywordsString().empty());
  CPPUNIT_ASSERT(m_Lexer->IsKeyword("class"));
  CPPUNIT_ASSERT(m_Lexer->IsKeyword("const"));
  CPPUNIT_ASSERT(m_Lexer->KeywordStartsWith("cla"));
  CPPUNIT_ASSERT(!m_Lexer->KeywordStartsWith("xxx"));
  CPPUNIT_ASSERT(!m_Lexer->MakeComment("test", true).empty());
  CPPUNIT_ASSERT(!m_Lexer->MakeComment("test", "test").empty());
  CPPUNIT_ASSERT(m_Lexer->SetKeywords("hello:1"));
  CPPUNIT_ASSERT(m_Lexer->SetKeywords("test11 test21:1 test31:1 test12:2 test22:2"));
  CPPUNIT_ASSERT(!m_Lexer->IsKeyword("class")); // now overwritten
  CPPUNIT_ASSERT(m_Lexer->IsKeyword("test11"));
  CPPUNIT_ASSERT(m_Lexer->IsKeyword("test21"));
  CPPUNIT_ASSERT(m_Lexer->IsKeyword("test12"));
  CPPUNIT_ASSERT(m_Lexer->IsKeyword("test22"));
  CPPUNIT_ASSERT(m_Lexer->KeywordStartsWith("te"));
  CPPUNIT_ASSERT(!m_Lexer->KeywordStartsWith("xx"));
  CPPUNIT_ASSERT(!m_Lexer->GetKeywords().empty());
    delete m_Lexer;
}

void wxExTestFixture::testLexers()
{
  wxExLexers* m_Lexers;
  m_Lexers = new wxExLexers(wxFileName("../extension/data/lexers.xml"));
  
  CPPUNIT_ASSERT(!m_Lexers->BuildWildCards(wxFileName(TEST_FILE)).empty());
  CPPUNIT_ASSERT(m_Lexers->Count() > 0);
  CPPUNIT_ASSERT(m_Lexers->FindByFileName(wxFileName(TEST_FILE)).GetScintillaLexer() == "cpp");
  CPPUNIT_ASSERT(m_Lexers->FindByName("cpp").GetScintillaLexer() == "cpp");
  CPPUNIT_ASSERT(m_Lexers->FindByText("// this is a cpp comment text").GetScintillaLexer() == "cpp");
}

void wxExTestFixture::testRCS()
{
  m_RCS = new wxExRCS();
  wxExRCS* m_RCS;
  CPPUNIT_ASSERT(m_RCS->GetDescription().empty());
  CPPUNIT_ASSERT(m_RCS->GetUser().empty());
  delete m_RCS;
}

void wxExTestFixture::testStat()
{
  wxExStat* m_Stat;
  m_Stat = new wxExStat(TEST_FILE);
  CPPUNIT_ASSERT(m_Stat->IsOk());
  CPPUNIT_ASSERT(!m_Stat->IsReadOnly());
  CPPUNIT_ASSERT(m_Stat->Sync("./test-base.link"));
  delete m_Stat;
}

void wxExTestFixture::testStatistics()
{
  wxExStatistics<long>* m_Statistics;
  m_Statistics = new wxExStatistics<long>();
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
  wxExStatistics<long> copy(*m_Statistics);
  CPPUNIT_ASSERT(copy.Get("test2") == 1);
  m_Statistics->Clear();
  CPPUNIT_ASSERT(m_Statistics->GetItems().empty());
  delete m_Statistics;
}

void wxExTestFixture::testTextFile()
{
  wxExTextFile textFile(wxExFileName(TEST_FILE), ID_TOOL_REPORT_COUNT);
  CPPUNIT_ASSERT(textFile.RunTool());
  CPPUNIT_ASSERT(!textFile.GetStatistics().GetElements().GetItems().empty());
  CPPUNIT_ASSERT(!textFile.IsOpened()); // file should be closed after running tool

  CPPUNIT_ASSERT(textFile.RunTool()); // do the same test
  CPPUNIT_ASSERT(!textFile.GetStatistics().GetElements().GetItems().empty());
  CPPUNIT_ASSERT(!textFile.IsOpened()); // file should be closed after running tool

  wxExTextFile textFile2(wxExFileName(TEST_FILE), ID_TOOL_REPORT_KEYWORD);
  CPPUNIT_ASSERT(textFile2.RunTool());
//  CPPUNIT_ASSERT(!m_TextFile->GetStatistics().GetKeywords().GetItems().empty());
}

void wxExTestFixture::testTiming()
{
  wxExFileTest file(TEST_FILE);

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

  printf(
    "wxExFile::Read:%ld wxFile::Read:%ld\n",
    exfile_read,
    file_read);
}

void wxExTestFixture::testTimingAttrib()
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

  printf(
    "wxExFileName::IsReadOnly:%ld wxFileName::IsFileWritable:%ld\n",
    exfile_time,
    file_time);
}

void wxExTestFixture::testTool()
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
  addTest(new CppUnit::TestCaller<wxExTestFixture>(
    "testMethods",
    &wxExTestFixture::testMethods));

  addTest(new CppUnit::TestCaller<wxExTestFixture>(
    "testTiming",
    &wxExTestFixture::testTiming));

  addTest(new CppUnit::TestCaller<wxExTestFixture>(
    "testTimingAttrib",
    &wxExTestFixture::testTimingAttrib));
}
