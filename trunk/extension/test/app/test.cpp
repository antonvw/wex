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

void wxExAppTestFixture::setUp()
{
  m_Grid = new wxExGrid(wxTheApp->GetTopWindow());
  m_ListView = new wxExListView(wxTheApp->GetTopWindow());
  m_Notebook = new wxExNotebook(wxTheApp->GetTopWindow(), NULL);
  m_STC = new wxExSTC(wxTheApp->GetTopWindow(), wxExFileName("test.h"));
  m_STCShell = new wxExSTCShell(wxTheApp->GetTopWindow());
  m_SVN = new wxExSVN(SVN_STAT, "test.h");
}

void wxExAppTestFixture::testConstructors()
{
}

void wxExAppTestFixture::testMethods()
{
  // test wxExApp
  CPPUNIT_ASSERT(wxExApp::GetConfig() != NULL);
  CPPUNIT_ASSERT(wxExApp::GetLexers() != NULL);
  CPPUNIT_ASSERT(wxExApp::GetPrinter() != NULL);
  CPPUNIT_ASSERT(!wxExTool::GetToolInfo().empty());

  // test wxExGrid
  CPPUNIT_ASSERT(m_Grid->CreateGrid(5, 5));
  m_Grid->SelectAll();
  m_Grid->SetGridCellValue(wxGridCellCoords(0, 0), "test");
  CPPUNIT_ASSERT(m_Grid->GetCellValue(0, 0) == "test");
  m_Grid->SetCellsValue(wxGridCellCoords(0, 0), "test1\ttest2\ntest3\ttest4\n");
  CPPUNIT_ASSERT(m_Grid->GetCellValue(0, 0) == "test1");

  // test wxExListView
  m_ListView->SetSingleStyle(wxLC_REPORT); // wxLC_ICON);
  m_ListView->InsertColumn("String", wxExColumn::COL_STRING);
  m_ListView->InsertColumn("Number", wxExColumn::COL_INT);
  CPPUNIT_ASSERT(m_ListView->FindColumn("String") == 0);
  CPPUNIT_ASSERT(m_ListView->FindColumn("Number") == 1);

  // test wxExNotebook (parent should not be NULL)
  wxWindow* page1 = new wxWindow(wxTheApp->GetTopWindow(), wxID_ANY);
  wxWindow* page2 = new wxWindow(wxTheApp->GetTopWindow(), wxID_ANY);

  CPPUNIT_ASSERT(m_Notebook->AddPage(page1, "key1") != NULL);
  CPPUNIT_ASSERT(m_Notebook->AddPage(page2, "key2") != NULL);
  CPPUNIT_ASSERT(m_Notebook->AddPage(page1, "key1") == NULL);
  CPPUNIT_ASSERT(m_Notebook->GetKeyByPage(page1) == "key1");
  CPPUNIT_ASSERT(m_Notebook->GetPageByKey("key1") == page1);
  CPPUNIT_ASSERT(m_Notebook->SetPageText("key1", "keyx", "hello"));
  CPPUNIT_ASSERT(m_Notebook->GetPageByKey("keyx") == page1);
  CPPUNIT_ASSERT(m_Notebook->DeletePage("keyx"));
  CPPUNIT_ASSERT(m_Notebook->GetPageByKey("keyx") == NULL);

  // test wxExSTC
  CPPUNIT_ASSERT(m_STC->GetFileName().GetFullName() == "test.h");
  // do the same test as with wxExFile in base for a binary file
  CPPUNIT_ASSERT(m_STC->Open(wxExFileName("../base/test.bin")));
  const wxCharBuffer& buffer = m_STC->GetTextRaw();
  wxLogMessage(buffer.data());
  CPPUNIT_ASSERT(buffer.length() == 40);

  // test wxExSTCShell
  m_STCShell->Prompt("test1");
  m_STCShell->Prompt("test2");
  m_STCShell->Prompt("test3");
  m_STCShell->Prompt("test4");
  // Prompting does not add a command to history...
  // TODO: Make a better test.
  CPPUNIT_ASSERT(!m_STCShell->GetHistory().Contains("test4"));

  // test wxExSVN
  CPPUNIT_ASSERT(m_SVN->Execute(false) == 0); // do not use a dialog
  // The output depends on the svn stat, of course,
  // so do not assert on it.
  m_SVN->GetOutput();

  // test various wxEx methods that need the app
  const wxString header = wxExHeader(wxExFileName("test.h"), m_Config, "hello test");
  CPPUNIT_ASSERT(header.Contains("hello test"));
  
  // test util
  CPPUNIT_ASSERT(wxExClipboardAdd("test"));
  CPPUNIT_ASSERT(wxExClipboardGet() == "test");
  CPPUNIT_ASSERT(wxExGetNumberOfLines("test\ntest\n") == 3);
  CPPUNIT_ASSERT(wxExGetLineNumberFromText("test on line: 1200") == 1200);
  CPPUNIT_ASSERT(wxExLog("hello from wxextension test"));
  CPPUNIT_ASSERT(!wxExMatchesOneOf(wxFileName("test.txt"), "*.cpp"));
  CPPUNIT_ASSERT(wxExMatchesOneOf(wxFileName("test.txt"), "*.cpp;*.txt"));
  CPPUNIT_ASSERT(wxExSkipWhiteSpace("t     es   t") == "t es t");
}

void wxExAppTestFixture::tearDown()
{
}

wxExTestSuite::wxExTestSuite()
  : CppUnit::TestSuite("wxextension test suite")
{
  addTest(new CppUnit::TestCaller<wxExAppTestFixture>(
    "testConstructors",
    &wxExAppTestFixture::testConstructors));

  addTest(new CppUnit::TestCaller<wxExAppTestFixture>(
    "testMethods",
    &wxExAppTestFixture::testMethods));
}
