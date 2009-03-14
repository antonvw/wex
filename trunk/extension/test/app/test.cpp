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

void exAppTestFixture::setUp()
{
  m_Dir = new exDir("./");
  m_Grid = new exGrid(wxTheApp->GetTopWindow());
  m_ListView = new exListView(wxTheApp->GetTopWindow());
  m_STC = new exSTC(wxTheApp->GetTopWindow(), exFileName("test.h"));
  m_STCShell = new exSTCShell(wxTheApp->GetTopWindow());
  m_SVN = new exSVN(SVN_STAT, "test.h");
}

void exAppTestFixture::testConstructors()
{
}

void exAppTestFixture::testMethods()
{
  // test exApp
  CPPUNIT_ASSERT(exApp::GetConfig() != NULL);
  CPPUNIT_ASSERT(exApp::GetLexers() != NULL);
  CPPUNIT_ASSERT(exApp::GetPrinter() != NULL);

  // test exDir
  CPPUNIT_ASSERT(m_Dir->FindFiles() > 0);
  CPPUNIT_ASSERT(m_Dir->GetFiles().GetCount() > 0);

  // test exGrid
  CPPUNIT_ASSERT(m_Grid->CreateGrid(5, 5));
  m_Grid->SelectAll();
  m_Grid->SetGridCellValue(wxGridCellCoords(0, 0), "test");
  CPPUNIT_ASSERT(m_Grid->GetCellValue(0, 0) == "test");
  m_Grid->SetCellsValue(wxGridCellCoords(0, 0), "test1\ttest2\ntest3\ttest4\n");
  CPPUNIT_ASSERT(m_Grid->GetCellValue(0, 0) == "test1");

  // test exListView
  m_ListView->SetSingleStyle(wxLC_REPORT); // wxLC_ICON);
  m_ListView->InsertColumn("String", exColumn::COL_STRING);
  m_ListView->InsertColumn("Number", exColumn::COL_INT);
  CPPUNIT_ASSERT(m_ListView->FindColumn("String") == 0);
  CPPUNIT_ASSERT(m_ListView->FindColumn("Number") == 1);

  // test exSTC
  CPPUNIT_ASSERT(m_STC->GetFileName().GetFullName() == "test.h");
  // do the same test as with exFile in base for a bianry file
  CPPUNIT_ASSERT(m_STC->Open(exFileName("../base/test.bin")));
  wxString* buffer = m_STC->GetTextRaw();
  wxLogMessage(*buffer);
  CPPUNIT_ASSERT(buffer != NULL);
  CPPUNIT_ASSERT(buffer->size() == 40);
  delete buffer;

  // test exSTCShell
  m_STCShell->Prompt("test1");
  m_STCShell->Prompt("test2");
  m_STCShell->Prompt("test3");
  m_STCShell->Prompt("test4");
  // Prompting does not add a command to history...
  // TODO: Make a better test.
  CPPUNIT_ASSERT(!m_STCShell->GetHistory().Contains("test4"));

  // test exSVN
  CPPUNIT_ASSERT(m_SVN->GetInfo(false) == 0); // do not use a dialog
  // The contents depends on the svn stat, of course,
  // so do not assert on it.
  m_SVN->GetContents();

  // test util
  CPPUNIT_ASSERT(exClipboardAdd("test"));
  CPPUNIT_ASSERT(exClipboardGet() == "test");
  CPPUNIT_ASSERT(exGetNumberOfLines("test\ntest\n") == 3);
  CPPUNIT_ASSERT(exGetLineNumberFromText("test on line: 1200") == 1200);
  CPPUNIT_ASSERT(exLog("hello from wxextension test"));
  CPPUNIT_ASSERT(!exMatchesOneOf(wxFileName("test.txt"), "*.cpp"));
  CPPUNIT_ASSERT(exMatchesOneOf(wxFileName("test.txt"), "*.cpp;*.txt"));
  CPPUNIT_ASSERT(exSkipWhiteSpace("t     es   t") == "t es t");
}

void exAppTestFixture::tearDown()
{
}

exTestSuite::exTestSuite()
  : CppUnit::TestSuite("wxextension test suite")
{
  addTest(new CppUnit::TestCaller<exAppTestFixture>(
    "testConstructors",
    &exAppTestFixture::testConstructors));

  addTest(new CppUnit::TestCaller<exAppTestFixture>(
    "testMethods",
    &exAppTestFixture::testMethods));
}
