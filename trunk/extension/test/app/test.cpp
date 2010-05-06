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
#include <wx/config.h>
#include <wx/extension/header.h>
#include <wx/extension/lexers.h>
#include <wx/extension/log.h>
#include <wx/extension/printing.h>
#include <wx/extension/tool.h>
#include "test.h"

#define TEST_FILE "./test.h"
#define TEST_BIN "./test.bin"

void wxExAppTestFixture::setUp()
{
  m_Grid = new wxExGrid(wxTheApp->GetTopWindow());
  m_ListView = new wxExListView(wxTheApp->GetTopWindow());
  m_Notebook = new wxExNotebook(wxTheApp->GetTopWindow(), NULL);
  m_STC = new wxExSTC(wxTheApp->GetTopWindow(), wxExFileName(TEST_FILE));
  m_STCShell = new wxExSTCShell(wxTheApp->GetTopWindow());
  m_VCS = new wxExVCS(VCS_INFO, TEST_FILE);
}

void wxExAppTestFixture::testConstructors()
{
}

void wxExAppTestFixture::testMethods()
{
  // test global objects
  CPPUNIT_ASSERT(wxExFindReplaceData::Get() != NULL);
  CPPUNIT_ASSERT(wxExLexers::Get() != NULL);
  CPPUNIT_ASSERT(wxExLog::Get() != NULL);
  CPPUNIT_ASSERT(wxExPrinting::Get() != NULL);
  CPPUNIT_ASSERT(wxExVCS::Get() != NULL);
  CPPUNIT_ASSERT(wxExTool::Get() != NULL);

  // test wxExGrid
  CPPUNIT_ASSERT(m_Grid->CreateGrid(5, 5));
  m_Grid->SetGridCellValue(wxGridCellCoords(0, 0), "test");
  m_Grid->SelectAll();
  CPPUNIT_ASSERT(!m_Grid->GetSelectedCellsValue().empty());
  CPPUNIT_ASSERT(m_Grid->GetCellValue(0, 0) == "test");
  m_Grid->SetCellsValue(wxGridCellCoords(0, 0), "test1\ttest2\ntest3\ttest4\n");
  CPPUNIT_ASSERT(m_Grid->GetCellValue(0, 0) == "test1");

  // test wxExListView
  m_ListView->SetSingleStyle(wxLC_REPORT); // wxLC_ICON);
  m_ListView->InsertColumn(wxExColumn("String", wxExColumn::COL_STRING));
  m_ListView->InsertColumn(wxExColumn("Number", wxExColumn::COL_INT));
  CPPUNIT_ASSERT(m_ListView->FindColumn("String") == 0);
  CPPUNIT_ASSERT(m_ListView->FindColumn("Number") == 1);
  wxExListItem item1(m_ListView, "c item"); ///< testing wxExListItem
  item1.Insert();
  wxExListItem item2(m_ListView, "b item"); ///< testing wxExListItem
  item2.Insert();
  wxExListItem item3(m_ListView, "a item"); ///< testing wxExListItem
  item3.Insert();
  m_ListView->SortColumn("String", SORT_ASCENDING);
  wxExListItem test(m_ListView, 0);
  CPPUNIT_ASSERT(test.GetColumnText("String") == "a item");

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
  // do the same test as with wxExFile in base for a binary file
  CPPUNIT_ASSERT(m_STC->Open(wxExFileName(TEST_BIN)));
  CPPUNIT_ASSERT(m_STC->GetFlags() == 0);
  CPPUNIT_ASSERT(m_STC->GetMenuFlags() == wxExSTC::STC_MENU_DEFAULT);
  const wxCharBuffer& buffer = m_STC->GetTextRaw();
  wxLogMessage(buffer.data());
  CPPUNIT_ASSERT(buffer.length() == 40);
  CPPUNIT_ASSERT(!m_STC->MacroIsRecording());
  CPPUNIT_ASSERT(!m_STC->MacroIsRecorded());
  m_STC->StartRecord();
  CPPUNIT_ASSERT( m_STC->MacroIsRecording());
  CPPUNIT_ASSERT(!m_STC->MacroIsRecorded());
  m_STC->StopRecord();
  CPPUNIT_ASSERT(!m_STC->MacroIsRecording());
  CPPUNIT_ASSERT(!m_STC->MacroIsRecorded()); // still no macro

  // test wxExSTCShell
  m_STCShell->Prompt("test1");
  m_STCShell->Prompt("test2");
  m_STCShell->Prompt("test3");
  m_STCShell->Prompt("test4");
  // Prompting does not add a command to history.
  CPPUNIT_ASSERT(!m_STCShell->GetHistory().Contains("test4"));
  // Post 3 'a' chars to the shell, and check whether it comes in the history.
  wxKeyEvent event(wxEVT_CHAR);
  event.m_keyCode = 97; // one char 'a'
  wxPostEvent(m_STCShell, event);
  wxPostEvent(m_STCShell, event);
  wxPostEvent(m_STCShell, event);
  event.m_keyCode = WXK_RETURN;
  wxPostEvent(m_STCShell, event);
  // The event queue for shell is not yet processed, so next will assert anyway.
  //CPPUNIT_ASSERT(m_STCShell->GetHistory().Contains("aaa"));

  // test wxExVCS
//  CPPUNIT_ASSERT(m_VCS->Execute(NULL) == 0); // do not use a dialog
//  CPPUNIT_ASSERT(!m_VCS->GetOutput().empty());

  // test util
  CPPUNIT_ASSERT(wxExClipboardAdd("test"));
  CPPUNIT_ASSERT(wxExClipboardGet() == "test");
  CPPUNIT_ASSERT(wxExGetNumberOfLines("test\ntest\n") == 3);
  CPPUNIT_ASSERT(wxExGetLineNumberFromText("test on line: 1200") == 1200);

  // Only usefull if the lexers file was present
  if (wxExLexers::Get()->Count() > 0)
  {
    wxConfigBase::Get()->Write(_("Purpose"), "hello test");
    const wxExFileName fn(TEST_FILE);
    const wxString header = wxExHeader().Get(&fn);
    CPPUNIT_ASSERT(header.Contains("hello test"));
  }
  else
  {
    wxLogMessage("No lexers available");
  }

  wxExLog::Get()->Log("hello from wxExtension test");
  CPPUNIT_ASSERT(!wxExMatchesOneOf(wxFileName("test.txt"), "*.cpp"));
  CPPUNIT_ASSERT(wxExMatchesOneOf(wxFileName("test.txt"), "*.cpp;*.txt"));
  CPPUNIT_ASSERT(wxExSkipWhiteSpace("t     es   t") == "t es t");
  CPPUNIT_ASSERT(!wxExTranslate("hello @PAGENUM@ from @PAGESCNT@", 1, 2).Contains("@"));
}

void wxExAppTestFixture::tearDown()
{
}

wxExTestSuite::wxExTestSuite()
  : CppUnit::TestSuite("wxExtension test suite")
{
  addTest(new CppUnit::TestCaller<wxExAppTestFixture>(
    "testConstructors",
    &wxExAppTestFixture::testConstructors));

  addTest(new CppUnit::TestCaller<wxExAppTestFixture>(
    "testMethods",
    &wxExAppTestFixture::testMethods));
}
