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
#include "test.h"

#define TEST_FILE "./test.h"
#define TEST_BIN "./test.bin"

void wxExAppTestFixture::testConfigItem()
{
  wxExConfigItem spacer;
  CPPUNIT_ASSERT(spacer.GetType() == CONFIG_SPACER);
  CPPUNIT_ASSERT(!spacer.GetIsRequired());
  CPPUNIT_ASSERT(spacer.GetControl() == NULL);
  
  wxExConfigItem spin("spin", 1, 5);
  CPPUNIT_ASSERT(spin.GetType() == CONFIG_SPINCTRL);
  CPPUNIT_ASSERT(!spin.GetIsRequired());
  CPPUNIT_ASSERT(spin.GetControl() == NULL);
  
  wxExConfigItem spind("spin", 1.0, 5.0);
  CPPUNIT_ASSERT(spind.GetType() == CONFIG_SPINCTRL_DOUBLE);
  CPPUNIT_ASSERT(!spind.GetIsRequired());
  CPPUNIT_ASSERT(spind.GetControl() == NULL);
  
  wxExConfigItem str("string");
  CPPUNIT_ASSERT(str.GetType() == CONFIG_STRING);
  CPPUNIT_ASSERT(!str.GetIsRequired());
  CPPUNIT_ASSERT(str.GetControl() == NULL);
  
  wxExConfigItem i("int", CONFIG_INT);
  CPPUNIT_ASSERT(i.GetType() == CONFIG_INT);
  CPPUNIT_ASSERT(!i.GetIsRequired());
  CPPUNIT_ASSERT(i.GetControl() == NULL);
  
  wxGridSizer sizer(3);
  
  spacer.Layout(wxTheApp->GetTopWindow(), &sizer);
  CPPUNIT_ASSERT(spacer.GetControl() == NULL); //!
  
  spin.Layout(wxTheApp->GetTopWindow(), &sizer);
  CPPUNIT_ASSERT(spin.GetControl() != NULL);
  
  spind.Layout(wxTheApp->GetTopWindow(), &sizer);
  CPPUNIT_ASSERT(spind.GetControl() != NULL);
  
  str.Layout(wxTheApp->GetTopWindow(), &sizer);
  CPPUNIT_ASSERT(str.GetControl() != NULL);
  
  i.Layout(wxTheApp->GetTopWindow(), &sizer);
  CPPUNIT_ASSERT(i.GetControl() != NULL);
}

void wxExAppTestFixture::testFrd()
{
  wxExFindReplaceData* frd = wxExFindReplaceData::Get(); 
  
  frd->SetUseRegularExpression(true);

  frd->SetFindString("find1");
  frd->SetFindString("find2");
  frd->SetFindString("find[0-9]");
  
  CPPUNIT_ASSERT(!frd->GetFindStrings().empty());
  CPPUNIT_ASSERT( frd->GetRegularExpression().IsValid());
  CPPUNIT_ASSERT( frd->GetRegularExpression().Matches("find9"));
}

void wxExAppTestFixture::testGlobal()
{
  CPPUNIT_ASSERT(wxExFindReplaceData::Get() != NULL);
  CPPUNIT_ASSERT(wxExLexers::Get() != NULL);
  CPPUNIT_ASSERT(wxExLog::Get() != NULL);
  CPPUNIT_ASSERT(wxExPrinting::Get() != NULL);
  CPPUNIT_ASSERT(wxExVCS::Get() != NULL);
  CPPUNIT_ASSERT(wxExTool::Get() != NULL);
}

void wxExAppTestFixture::testGrid()
{
  wxExGrid* grid = new wxExGrid(wxTheApp->GetTopWindow());
  CPPUNIT_ASSERT(grid->CreateGrid(5, 5));
  grid->SetGridCellValue(wxGridCellCoords(0, 0), "test");
  grid->SelectAll();
  CPPUNIT_ASSERT(!grid->GetSelectedCellsValue().empty());
  CPPUNIT_ASSERT(grid->GetCellValue(0, 0) == "test");
  grid->SetCellsValue(wxGridCellCoords(0, 0), "test1\ttest2\ntest3\ttest4\n");
  CPPUNIT_ASSERT(grid->GetCellValue(0, 0) == "test1");
}

void wxExAppTestFixture::testHeader()
{
  wxExFileName filename(TEST_FILE);
  wxExHeader header;
  header.Set("hello test", "AvW");
  const wxString str = header.Get(&filename);
  CPPUNIT_ASSERT(!str.empty());
  CPPUNIT_ASSERT(str.Contains("hello test"));
  CPPUNIT_ASSERT(str.Contains("AvW"));
}

void wxExAppTestFixture::testLexer()
{
  wxExLexers lexers(wxFileName("../extension/data/lexers.xml"));
  wxExLexer lexer;
  lexer = lexers.FindByText("// this is a cpp comment text");
  CPPUNIT_ASSERT(lexer.GetScintillaLexer().empty());
  
  // now read lexers
  lexers.Read();
  lexer = lexers.FindByText("// this is a cpp comment text");
  
  CPPUNIT_ASSERT(!lexer.GetExtensions().empty());
  CPPUNIT_ASSERT(!lexer.GetCommentBegin().empty());
  CPPUNIT_ASSERT(!lexer.GetCommentBegin2().empty());
  CPPUNIT_ASSERT(lexer.GetCommentEnd().empty());
  CPPUNIT_ASSERT(!lexer.GetCommentEnd2().empty());
  CPPUNIT_ASSERT(!lexer.GetKeywords().empty());
  CPPUNIT_ASSERT(!lexer.GetKeywordsString().empty());
  CPPUNIT_ASSERT(lexer.IsKeyword("class"));
  CPPUNIT_ASSERT(lexer.IsKeyword("const"));
  CPPUNIT_ASSERT(lexer.KeywordStartsWith("cla"));
  CPPUNIT_ASSERT(!lexer.KeywordStartsWith("xxx"));
  CPPUNIT_ASSERT(!lexer.MakeComment("test", true).empty());
  CPPUNIT_ASSERT(!lexer.MakeComment("test", "test").empty());
  CPPUNIT_ASSERT(lexer.SetKeywords("hello:1"));
  CPPUNIT_ASSERT(lexer.SetKeywords("test11 test21:1 test31:1 test12:2 test22:2"));
  CPPUNIT_ASSERT(!lexer.IsKeyword("class")); // now overwritten
  CPPUNIT_ASSERT(lexer.IsKeyword("test11"));
  CPPUNIT_ASSERT(lexer.IsKeyword("test21"));
  CPPUNIT_ASSERT(lexer.IsKeyword("test12"));
  CPPUNIT_ASSERT(lexer.IsKeyword("test22"));
  CPPUNIT_ASSERT(lexer.KeywordStartsWith("te"));
  CPPUNIT_ASSERT(!lexer.KeywordStartsWith("xx"));
  CPPUNIT_ASSERT(!lexer.GetKeywords().empty());
}

void wxExAppTestFixture::testLexers()
{
  wxExLexers lexers(wxFileName("../extension/data/lexers.xml"));
  lexers.Read();
  
  CPPUNIT_ASSERT(!lexers.BuildWildCards(wxFileName(TEST_FILE)).empty());
  CPPUNIT_ASSERT(lexers.Count() > 0);
  CPPUNIT_ASSERT(lexers.FindByFileName(wxFileName(TEST_FILE)).GetScintillaLexer() == "cpp");
  CPPUNIT_ASSERT(lexers.FindByName("cpp").GetScintillaLexer() == "cpp");
  CPPUNIT_ASSERT(lexers.FindByText("// this is a cpp comment text").GetScintillaLexer() == "cpp");
  CPPUNIT_ASSERT(lexers.FindByName("xxx").GetScintillaLexer().empty());
  CPPUNIT_ASSERT(!lexers.GetMacros().empty());
  CPPUNIT_ASSERT(!lexers.GetMacrosStyle().empty());
}

void wxExAppTestFixture::testListView()
{
  wxExListView* listView = new wxExListView(wxTheApp->GetTopWindow());
  
  listView->InsertColumn(wxExColumn("String", wxExColumn::COL_STRING));
  listView->InsertColumn(wxExColumn("Number", wxExColumn::COL_INT));
  CPPUNIT_ASSERT(listView->FindColumn("String")  == 0);
  CPPUNIT_ASSERT(listView->FindColumn("Number") == 1);
}

void wxExAppTestFixture::testLog()
{
  CPPUNIT_ASSERT(!wxExLog::Get()->GetFileName().GetFullPath().empty());
  CPPUNIT_ASSERT(!wxExLog::Get()->GetLogging());
  
  wxExLog log(wxFileName("output.log"));
  CPPUNIT_ASSERT(log.Log("hello from wxExtension test"));
}

void wxExAppTestFixture::testMenu()
{
  wxExMenu menu;
  
  menu.AppendSeparator();
  menu.AppendSeparator();
  menu.AppendSeparator();
  menu.AppendSeparator();
  CPPUNIT_ASSERT(menu.GetItemsAppended() == 0);
  
  menu.AppendBars();
  CPPUNIT_ASSERT(menu.GetItemsAppended() > 0);
  
  CPPUNIT_ASSERT(!menu.IsVCSBuild());
  
  menu.BuildVCS(true);
  CPPUNIT_ASSERT(menu.IsVCSBuild());
  
  menu.BuildVCS(false);
  CPPUNIT_ASSERT(!menu.IsVCSBuild());
}

void wxExAppTestFixture::testNotebook()
{
  wxExNotebook* notebook = new wxExNotebook(wxTheApp->GetTopWindow(), NULL);
  // (parent should not be NULL)
  wxWindow* page1 = new wxWindow(wxTheApp->GetTopWindow(), wxID_ANY);
  wxWindow* page2 = new wxWindow(wxTheApp->GetTopWindow(), wxID_ANY);
  CPPUNIT_ASSERT(notebook->AddPage(page1, "key1") != NULL);
  CPPUNIT_ASSERT(notebook->AddPage(page2, "key2") != NULL);
  CPPUNIT_ASSERT(notebook->AddPage(page1, "key1") == NULL);
  CPPUNIT_ASSERT(notebook->GetKeyByPage(page1) == "key1");
  CPPUNIT_ASSERT(notebook->GetPageByKey("key1") == page1);
  CPPUNIT_ASSERT(notebook->SetPageText("key1", "keyx", "hello"));
  CPPUNIT_ASSERT(notebook->GetPageByKey("keyx") == page1);
  CPPUNIT_ASSERT(notebook->DeletePage("keyx"));
  CPPUNIT_ASSERT(notebook->GetPageByKey("keyx") == NULL);
}

void wxExAppTestFixture::testSTC()
{
  wxExSTC* stc = new wxExSTC(wxTheApp->GetTopWindow(), "hello stc");
  CPPUNIT_ASSERT(stc->GetText() == "hello stc");
  
  stc->AppendTextForced("more text");
  wxLogMessage(stc->GetText());
  CPPUNIT_ASSERT(stc->GetText() != "hello stc");
  // next asserts, do not know why
//  CPPUNIT_ASSERT(stc->FindNext("more text"));
  
  stc->ReplaceAll("more", "less");
//  CPPUNIT_ASSERT(stc->FindNext("less text"));
  
  stc->SetText("new text");
  CPPUNIT_ASSERT(stc->GetText() == "new text");
  
  CPPUNIT_ASSERT(!stc->MacroIsRecording());
  CPPUNIT_ASSERT(!stc->MacroIsRecorded());
  
  stc->StartRecord();
  CPPUNIT_ASSERT( stc->MacroIsRecording());
  CPPUNIT_ASSERT(!stc->MacroIsRecorded());
  
  stc->StopRecord();
  CPPUNIT_ASSERT(!stc->MacroIsRecording());
  CPPUNIT_ASSERT(!stc->MacroIsRecorded()); // still no macro
}
  
void wxExAppTestFixture::testSTCFile()
{
  wxExSTCFile* stc = new wxExSTCFile(wxTheApp->GetTopWindow(), wxExFileName(TEST_FILE));
  
  // do the same test as with wxExFile in base for a binary file
  CPPUNIT_ASSERT(stc->Open(wxExFileName(TEST_BIN)));
  CPPUNIT_ASSERT(stc->GetFlags() == 0);
  CPPUNIT_ASSERT(stc->GetMenuFlags() == wxExSTCFile::STC_MENU_DEFAULT);
  const wxCharBuffer& buffer = stc->GetTextRaw();
  CPPUNIT_ASSERT(buffer.length() == 40);
}

void wxExAppTestFixture::testSTCShell()
{
  wxExSTCShell* shell = new wxExSTCShell(wxTheApp->GetTopWindow());
  shell->Prompt("test1");
  shell->Prompt("test2");
  shell->Prompt("test3");
  shell->Prompt("test4");
  // Prompting does not add a command to history.
  CPPUNIT_ASSERT(!shell->GetHistory().Contains("test4"));
  // Post 3 'a' chars to the shell, and check whether it comes in the history.
  wxKeyEvent event(wxEVT_CHAR);
  event.m_keyCode = 97; // one char 'a'
  wxPostEvent(shell, event);
  wxPostEvent(shell, event);
  wxPostEvent(shell, event);
  event.m_keyCode = WXK_RETURN;
  wxPostEvent(shell, event);
  // The event queue for shell is not yet processed, so next will assert anyway.
  //CPPUNIT_ASSERT(m_STCShell->GetHistory().Contains("aaa"));
}

void wxExAppTestFixture::testUtil()
{
  CPPUNIT_ASSERT(wxExClipboardAdd("test"));
  CPPUNIT_ASSERT(wxExClipboardGet() == "test");
  CPPUNIT_ASSERT(wxExGetNumberOfLines("test\ntest\n") == 3);
  CPPUNIT_ASSERT(wxExGetLineNumberFromText("test on line: 1200") == 1200);
  CPPUNIT_ASSERT(!wxExMatchesOneOf(wxFileName("test.txt"), "*.cpp"));
  CPPUNIT_ASSERT(wxExMatchesOneOf(wxFileName("test.txt"), "*.cpp;*.txt"));
  CPPUNIT_ASSERT(wxExSkipWhiteSpace("\n\tt \n    es   t\n") == "t es t");
  CPPUNIT_ASSERT(!wxExTranslate("hello @PAGENUM@ from @PAGESCNT@", 1, 2).Contains("@"));
}

void wxExAppTestFixture::testVCS()
{
  wxExVCS vcs(wxExVCS::VCS_INFO, TEST_FILE);
  // There is a problem in wxExecute inside wxExVCS::Execute.
//  CPPUNIT_ASSERT(m_VCS->Execute() != -1);
//  CPPUNIT_ASSERT(!m_VCS->GetOutput().empty());
  CPPUNIT_ASSERT(vcs.DirExists(wxFileName(TEST_FILE)));
}

void wxExAppTestFixture::testVi()
{
  wxExSTCFile* stc = new wxExSTCFile(wxTheApp->GetTopWindow(), wxExFileName(TEST_FILE));
  wxExVi* vi = new wxExVi(stc);
  
  CPPUNIT_ASSERT(!vi->GetActive());
  
  vi->Use(true);
  CPPUNIT_ASSERT(vi->GetActive());
  
  wxKeyEvent event(wxEVT_CHAR);
  event.m_keyCode = 97; // one char 'a'
  
  CPPUNIT_ASSERT(!vi->OnChar(event));
}
  
wxExTestSuite::wxExTestSuite()
  : CppUnit::TestSuite("wxExtension test suite")
{
  addTest(new CppUnit::TestCaller<wxExAppTestFixture>(
    "testConfigItem",
    &wxExAppTestFixture::testConfigItem));
    
  addTest(new CppUnit::TestCaller<wxExAppTestFixture>(
    "testFrd",
    &wxExAppTestFixture::testFrd));
    
  addTest(new CppUnit::TestCaller<wxExAppTestFixture>(
    "testGlobal",
    &wxExAppTestFixture::testGlobal));
    
  addTest(new CppUnit::TestCaller<wxExAppTestFixture>(
    "testGrid",
    &wxExAppTestFixture::testGrid));
    
  addTest(new CppUnit::TestCaller<wxExAppTestFixture>(
    "testHeader",
    &wxExAppTestFixture::testHeader));
    
  addTest(new CppUnit::TestCaller<wxExAppTestFixture>(
    "testLexer",
    &wxExAppTestFixture::testLexer));

  addTest(new CppUnit::TestCaller<wxExAppTestFixture>(
    "testLexers",
    &wxExAppTestFixture::testLexers));

  addTest(new CppUnit::TestCaller<wxExAppTestFixture>(
    "testListView",
    &wxExAppTestFixture::testListView));
    
  addTest(new CppUnit::TestCaller<wxExAppTestFixture>(
    "testLog",
    &wxExAppTestFixture::testLog));
    
  addTest(new CppUnit::TestCaller<wxExAppTestFixture>(
    "testMenu",
    &wxExAppTestFixture::testMenu));
    
  addTest(new CppUnit::TestCaller<wxExAppTestFixture>(
    "testNotebook",
    &wxExAppTestFixture::testNotebook));
    
  addTest(new CppUnit::TestCaller<wxExAppTestFixture>(
    "testSTC",
    &wxExAppTestFixture::testSTC));
    
  addTest(new CppUnit::TestCaller<wxExAppTestFixture>(
    "testSTCFile",
    &wxExAppTestFixture::testSTCFile));
    
  addTest(new CppUnit::TestCaller<wxExAppTestFixture>(
    "testSTCShell",
    &wxExAppTestFixture::testSTCShell));
    
  addTest(new CppUnit::TestCaller<wxExAppTestFixture>(
    "testUtil",
    &wxExAppTestFixture::testUtil));
    
  addTest(new CppUnit::TestCaller<wxExAppTestFixture>(
    "testVCS",
    &wxExAppTestFixture::testVCS));
    
  addTest(new CppUnit::TestCaller<wxExAppTestFixture>(
    "testVi",
    &wxExAppTestFixture::testVi));
}
