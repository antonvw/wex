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

#include <vector>
#include <TestCaller.h>
#include <wx/config.h>
#include "test.h"

#define TEST_FILE "./test.h"
#define TEST_BIN "./test.bin"

void wxExAppTestFixture::setUp()
{
  // Create the global lexers object, 
  // it should be present in ~/.wxex-test-app
  // (depending on platform, configuration).
  wxExLexers* lexers = wxExLexers::Get();
}

void wxExAppTestFixture::testConfigItem()
{
  std::vector <wxExConfigItem> items;

  // Use specific connstructors.
  wxExConfigItem ci1("ci1", 1, 5);
  items.push_back(ci1);
  CPPUNIT_ASSERT(ci1.GetName() == "ci1");
  CPPUNIT_ASSERT(ci1.GetType() == CONFIG_SPINCTRL);
  
  wxExConfigItem ci2("ci1", 1.0, 5.0);
  items.push_back(ci2);
  CPPUNIT_ASSERT(ci2.GetType() == CONFIG_SPINCTRL_DOUBLE);
  
  wxExConfigItem ci3("string");
  items.push_back(ci3);
  CPPUNIT_ASSERT(ci3.GetType() == CONFIG_STRING);
  
  wxExConfigItem ci4("int", CONFIG_INT);
  items.push_back(ci4);
  CPPUNIT_ASSERT(ci4.GetType() == CONFIG_INT);

  std::map<long, const wxString> echoices;
  echoices.insert(std::make_pair(0, _("Zero")));
  echoices.insert(std::make_pair(1, _("One")));
  echoices.insert(std::make_pair(2, _("Two")));
  wxExConfigItem ci5("Radio Box", echoices, true);
  items.push_back(ci5);
  CPPUNIT_ASSERT(ci5.GetType() == CONFIG_RADIOBOX);

  std::map<long, const wxString> cl;
  cl.insert(std::make_pair(0, _("Bit One")));
  cl.insert(std::make_pair(1, _("Bit Two")));
  cl.insert(std::make_pair(2, _("Bit Three")));
  cl.insert(std::make_pair(4, _("Bit Four")));
  wxExConfigItem ci6("Bin Choices", cl, false);
  items.push_back(ci6);
  CPPUNIT_ASSERT(ci6.GetType() == CONFIG_CHECKLISTBOX);

  std::set<wxString> bchoices;
  bchoices.insert(_("This"));
  bchoices.insert(_("Or"));
  bchoices.insert(_("Other"));
  wxExConfigItem ci7(bchoices);
  items.push_back(ci7);
  CPPUNIT_ASSERT(ci7.GetType() == CONFIG_CHECKLISTBOX_NONAME);

  // Use general constructor, and add all items.
  for (
    int i = CONFIG_ITEM_MIN + 1;
    i < CONFIG_ITEM_MAX;
    i++)
  {
    items.push_back(wxExConfigItem(wxString::Format("item%d", i), i));
  }

  // Check members are initialized.
  for (
    auto it = items.begin();
    it != items.end();
    ++it)
  {
    CPPUNIT_ASSERT(it->GetControl() == NULL);
    CPPUNIT_ASSERT(!it->GetIsRequired());
    CPPUNIT_ASSERT(it->GetColumns() == -1);
    CPPUNIT_ASSERT(it->GetPage().empty());
  }

  wxGridSizer sizer(3);

  // Layout the items and check control is created.
  for (
    auto it = items.begin();
    it != items.end();
    ++it)
  {
    it->Layout(wxTheApp->GetTopWindow(), &sizer);
    CPPUNIT_ASSERT(it->GetControl() != NULL);
  }
}

void wxExAppTestFixture::testFrame()
{
  wxExFrame* frame = (wxExFrame*)wxTheApp->GetTopWindow();

  std::vector<wxExPane> panes;

  panes.push_back(wxExPane("PaneText", -3));

  for (int i = 0; i < 25; i++)
  {
    panes.push_back(wxExPane(wxString::Format("Pane%d", i)));
  }
  
  CPPUNIT_ASSERT(frame->SetupStatusBar(panes) != NULL);
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

  frd->SetReplaceString("replace1");
  frd->SetReplaceString("replace2");
  frd->SetReplaceString("replace[0-9]");

  CPPUNIT_ASSERT(!frd->GetReplaceStrings().empty());
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
  CPPUNIT_ASSERT(str.Contains("Name"));
  CPPUNIT_ASSERT(str.Contains("Purpose"));
  CPPUNIT_ASSERT(str.Contains("Author"));
  CPPUNIT_ASSERT(str.Contains("Created"));
  CPPUNIT_ASSERT(str.Contains("Copyright"));
}

void wxExAppTestFixture::testLexer()
{
  wxExLexer lexer;
  lexer = wxExLexers::Get()->FindByText("// this is a cpp comment text");
  
  CPPUNIT_ASSERT(!lexer.GetExtensions().empty());
  CPPUNIT_ASSERT(!lexer.GetCommentBegin().empty());
  CPPUNIT_ASSERT(!lexer.GetCommentBegin2().empty());
  CPPUNIT_ASSERT( lexer.GetCommentEnd().empty());
  CPPUNIT_ASSERT(!lexer.GetCommentEnd2().empty());
  CPPUNIT_ASSERT(!lexer.GetKeywords().empty());
  CPPUNIT_ASSERT(!lexer.GetKeywordsString().empty());

  CPPUNIT_ASSERT( lexer.IsKeyword("class"));
  CPPUNIT_ASSERT( lexer.IsKeyword("const"));

  CPPUNIT_ASSERT( lexer.KeywordStartsWith("cla"));
  CPPUNIT_ASSERT(!lexer.KeywordStartsWith("xxx"));

  CPPUNIT_ASSERT(!lexer.MakeComment("test", true).empty());
  CPPUNIT_ASSERT(!lexer.MakeComment("test", "test").empty());

  CPPUNIT_ASSERT( lexer.SetKeywords("hello:1"));
  CPPUNIT_ASSERT( lexer.SetKeywords(
    "test11 test21:1 test31:1 test12:2 test22:2"));

  CPPUNIT_ASSERT(!lexer.IsKeyword("class")); // now overwritten
  CPPUNIT_ASSERT( lexer.IsKeyword("test11"));
  CPPUNIT_ASSERT( lexer.IsKeyword("test21"));
  CPPUNIT_ASSERT( lexer.IsKeyword("test12"));
  CPPUNIT_ASSERT( lexer.IsKeyword("test22"));

  CPPUNIT_ASSERT( lexer.KeywordStartsWith("te"));
  CPPUNIT_ASSERT(!lexer.KeywordStartsWith("xx"));

  CPPUNIT_ASSERT(!lexer.GetKeywords().empty());
}

void wxExAppTestFixture::testLexers()
{
  CPPUNIT_ASSERT( wxExLexers::Get()->ApplyMacro("XXX") == "XXX");
  CPPUNIT_ASSERT( wxExLexers::Get()->ApplyMacro("wxSTC_MARK_LCORNER") == "10");

  CPPUNIT_ASSERT(!wxExLexers::Get()->BuildWildCards(
    wxFileName(TEST_FILE)).empty());
  CPPUNIT_ASSERT( wxExLexers::Get()->Count() > 0);

  CPPUNIT_ASSERT( wxExLexers::Get()->FindByFileName(
    wxFileName(TEST_FILE)).GetScintillaLexer() == "cpp");
  CPPUNIT_ASSERT( wxExLexers::Get()->FindByName(
    "cpp").GetScintillaLexer() == "cpp");
  CPPUNIT_ASSERT( wxExLexers::Get()->FindByText(
    "// this is a cpp comment text").GetScintillaLexer() == "cpp");
  CPPUNIT_ASSERT( wxExLexers::Get()->FindByName(
    "xxx").GetScintillaLexer().empty());

  CPPUNIT_ASSERT( wxExLexers::Get()->GetDefaultStyle().IsDefault());
  CPPUNIT_ASSERT( wxExLexers::Get()->GetDefaultStyle().IsOk());

  CPPUNIT_ASSERT( wxExLexers::Get()->GetFileName().IsOk());

  CPPUNIT_ASSERT(!wxExLexers::Get()->GetMacros().empty());
  CPPUNIT_ASSERT(!wxExLexers::Get()->GetMacrosStyle().empty());

  CPPUNIT_ASSERT( wxExLexers::Get()->IndicatorIsLoaded(wxExIndicator(0)));
  CPPUNIT_ASSERT( wxExLexers::Get()->MarkerIsLoaded(wxExMarker(0)));
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
  CPPUNIT_ASSERT(menu.GetMenuItemCount() == 0);
  
  menu.AppendBars();
  CPPUNIT_ASSERT(menu.GetMenuItemCount() > 0);
  
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

void wxExAppTestFixture::testStatusBar()
{
  wxExFrame* frame = (wxExFrame*)wxTheApp->GetTopWindow();

  std::vector<wxExPane> panes;
  panes.push_back(wxExPane("PaneText", -3));
  panes.push_back(wxExPane("panex"));
  panes.push_back(wxExPane("paney"));
  panes.push_back(wxExPane("panez"));

  wxExStatusBar* sb = new wxExStatusBar(frame);
  
  // The next is OK, but asserts in wxWidgets.
  //../src/generic/statusbr.cpp(179): assert "(size_t)n == m_panes.GetCount()" 
  // failed in SetStatusWidths(): status bar field count mismatch
  //../src/common/statbar.cpp(189): assert "(size_t)n == m_panes.GetCount()" 
  // failed in SetStatusStyles(): field number mismatch
  //CPPUNIT_ASSERT(sb->SetPanes(panes) == panes.size());
  //CPPUNIT_ASSERT(sb->SetStatusText("hello"));
  //CPPUNIT_ASSERT(sb->SetStatusText("hello", "panex"));
  //CPPUNIT_ASSERT(sb->SetStatusText("hello", "paney"));
  //CPPUNIT_ASSERT(sb->SetStatusText("hello", "panez"));
  //CPPUNIT_ASSERT(!sb->SetStatusText("hello", "panexxx"));
}

void wxExAppTestFixture::testSTC()
{
  wxExSTC* stc = new wxExSTC(wxTheApp->GetTopWindow(), "hello stc");
  CPPUNIT_ASSERT(stc->GetText() == "hello stc");
  
  stc->AppendTextForced("more text");
  CPPUNIT_ASSERT(stc->GetText() != "hello stc");
  stc->DocumentStart();
  
  // next asserts, do not know why
//  CPPUNIT_ASSERT(stc->FindNext("more text"));
  
  CPPUNIT_ASSERT(stc->ReplaceAll("more", "less") == 1);
//  CPPUNIT_ASSERT(stc->FindNext("less text"));
  
  stc->SetText("new text");
  CPPUNIT_ASSERT(stc->GetText() == "new text");
  
  CPPUNIT_ASSERT(stc->SetLexer("cpp"));

  wxExLexer lexer;
  CPPUNIT_ASSERT(lexer.ApplyLexer("cpp", stc, false));
  CPPUNIT_ASSERT(!lexer.ApplyLexer("xyz", stc, false));
  
  CPPUNIT_ASSERT(!stc->MacroIsRecording());
  CPPUNIT_ASSERT(!stc->MacroIsRecorded());
  
  stc->StartRecord();
  CPPUNIT_ASSERT( stc->MacroIsRecording());
  CPPUNIT_ASSERT(!stc->MacroIsRecorded());
  
  stc->StopRecord();
  CPPUNIT_ASSERT(!stc->MacroIsRecording());
  CPPUNIT_ASSERT(!stc->MacroIsRecorded()); // still no macro
  
  // do the same test as with wxExFile in base for a binary file
  CPPUNIT_ASSERT(stc->Open(wxExFileName(TEST_BIN)));
  CPPUNIT_ASSERT(stc->GetFlags() == 0);
  const wxCharBuffer& buffer = stc->GetTextRaw();
  CPPUNIT_ASSERT(buffer.length() == 40);
}
  
void wxExAppTestFixture::testSTCFile()
{
  wxExSTC* stc = new wxExSTC(wxTheApp->GetTopWindow(), wxExFileName(TEST_FILE));
  wxExSTCFile file(stc);

  // The file itself is not assigned.  
  CPPUNIT_ASSERT(!file.GetFileName().GetStat().IsOk());
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
  CPPUNIT_ASSERT(
    wxExAlignText("test", "header", true, true,
      wxExLexers::Get()->FindByName("cpp")).size() 
      == wxString("// headertest").size());
  CPPUNIT_ASSERT(wxExClipboardAdd("test"));
  CPPUNIT_ASSERT(wxExClipboardGet() == "test");
  CPPUNIT_ASSERT(wxExGetEndOfText("test", 3).size() == 3);
  CPPUNIT_ASSERT(wxExGetEndOfText("testtest", 3).size() == 3);
  CPPUNIT_ASSERT(wxExGetLineNumber("test on line: 1200") == 1200);
  CPPUNIT_ASSERT(wxExGetNumberOfLines("test\ntest\n") == 3);
  CPPUNIT_ASSERT(!wxExMatchesOneOf(wxFileName("test.txt"), "*.cpp"));
  CPPUNIT_ASSERT(wxExMatchesOneOf(wxFileName("test.txt"), "*.cpp;*.txt"));
  CPPUNIT_ASSERT(wxExSkipWhiteSpace("\n\tt \n    es   t\n") == "t es t");
  CPPUNIT_ASSERT(!wxExTranslate(
    "hello @PAGENUM@ from @PAGESCNT@", 1, 2).Contains("@"));
}

void wxExAppTestFixture::testVCS()
{
  CPPUNIT_ASSERT( wxExVCS::Get()->DirExists(wxFileName(TEST_FILE)));
  wxMenu* menu = new wxMenu("test");
  wxExVCS::Get()->BuildMenu(100, menu);
  CPPUNIT_ASSERT(menu->GetMenuItemCount() > 0);
  CPPUNIT_ASSERT( wxExVCS::Get()->GetOutput().empty());
  CPPUNIT_ASSERT(!wxExVCS::Get()->SupportKeywordExpansion()); // TODO: why not
  CPPUNIT_ASSERT( wxExVCS::Get()->Use());

  // There is a problem in wxExecute inside wxExVCS::Execute (it hangs).
//  CPPUNIT_ASSERT(vcs->Execute() != -1);
//  CPPUNIT_ASSERT(!vcs->GetOutput().empty());
}

void wxExAppTestFixture::testVCSCommand()
{
  wxExVCSCommand::ResetInstances();
  
  const wxExVCSCommand add("a&dd");
  const wxExVCSCommand commit("commit", "main");
  const wxExVCSCommand diff("diff", "popup", "submenu");
  const wxExVCSCommand help("h&elp", "error", "", "m&e");
  const wxExVCSCommand open("blame");
  const wxExVCSCommand update("update");

  CPPUNIT_ASSERT(add.GetCommand() == "add");
  CPPUNIT_ASSERT(add.GetCommand(true, true) == "a&dd");
  CPPUNIT_ASSERT(help.GetCommand() == "help me");
  CPPUNIT_ASSERT(help.GetCommand(true, true) == "h&elp m&e");
  CPPUNIT_ASSERT(help.GetCommand(false, true) == "h&elp");
  CPPUNIT_ASSERT(help.GetCommand(false, false) == "help");
  
  CPPUNIT_ASSERT(add.GetNo() == 0);
  CPPUNIT_ASSERT(update.GetNo() == 5);

  CPPUNIT_ASSERT(add.GetType() == wxExVCSCommand::VCS_COMMAND_IS_BOTH);
  CPPUNIT_ASSERT(commit.GetType() == wxExVCSCommand::VCS_COMMAND_IS_MAIN);
  CPPUNIT_ASSERT(diff.GetType() == wxExVCSCommand::VCS_COMMAND_IS_POPUP);
  CPPUNIT_ASSERT(help.GetType() == wxExVCSCommand::VCS_COMMAND_IS_UNKNOWN);
  CPPUNIT_ASSERT(open.GetType() == wxExVCSCommand::VCS_COMMAND_IS_BOTH);

  CPPUNIT_ASSERT(add.IsAdd());
  CPPUNIT_ASSERT(commit.IsCommit());
  CPPUNIT_ASSERT(diff.IsDiff());
  CPPUNIT_ASSERT(help.IsHelp());
  CPPUNIT_ASSERT(open.IsOpen());
  CPPUNIT_ASSERT(update.IsUpdate());

  CPPUNIT_ASSERT(add.SubMenu().empty());
  CPPUNIT_ASSERT(diff.SubMenu() == "submenu");
  CPPUNIT_ASSERT(help.SubMenu() == "m&e");
}

void wxExAppTestFixture::testVCSEntry()
{
  wxExVCSEntry test;
  
  CPPUNIT_ASSERT(test.GetCommand(0).GetCommand().empty());
  CPPUNIT_ASSERT(test.GetName().empty());
  CPPUNIT_ASSERT(test.GetNo() == -1);
  CPPUNIT_ASSERT(!test.SupportKeywordExpansion());
}

void wxExAppTestFixture::testVi()
{
  wxExSTC* stc = new wxExSTC(wxTheApp->GetTopWindow(), wxExFileName(TEST_FILE));
  wxExVi* vi = new wxExVi(stc);
  
  CPPUNIT_ASSERT(!vi->GetIsActive());
  
  vi->Use(true);
  CPPUNIT_ASSERT(vi->GetIsActive());
  
  wxKeyEvent event(wxEVT_CHAR);
  event.m_keyCode = 97; // one char 'a'
  
  CPPUNIT_ASSERT(!vi->OnChar(event));
}
  
wxExAppTestSuite::wxExAppTestSuite()
  : CppUnit::TestSuite("wxExtension test suite")
{
  addTest(new CppUnit::TestCaller<wxExAppTestFixture>(
    "testConfigItem",
    &wxExAppTestFixture::testConfigItem));
    
  addTest(new CppUnit::TestCaller<wxExAppTestFixture>(
    "testFrame",
    &wxExAppTestFixture::testFrame));

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
    "testStatusBar",
    &wxExAppTestFixture::testStatusBar));

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
    "testVCSCommand",
    &wxExAppTestFixture::testVCSCommand));
    
  addTest(new CppUnit::TestCaller<wxExAppTestFixture>(
    "testVCSEntry",
    &wxExAppTestFixture::testVCSEntry));
    
  addTest(new CppUnit::TestCaller<wxExAppTestFixture>(
    "testVi",
    &wxExAppTestFixture::testVi));
}
