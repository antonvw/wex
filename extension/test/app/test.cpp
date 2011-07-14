////////////////////////////////////////////////////////////////////////////////
// Name:      test.cpp
// Purpose:   Implementation for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

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

void wxExAppTestFixture::testCommand()
{
  wxExCommand command;
  
  CPPUNIT_ASSERT(!command.GetError());
  CPPUNIT_ASSERT( command.GetOutput().empty());
  
  // wxExecute hangs, see also wxExVCS test
//  CPPUNIT_ASSERT( command.Execute("ls -l") != -1);
//  CPPUNIT_ASSERT(!command.GetError());
//  CPPUNIT_ASSERT(!command.GetOutput().empty());
  
//  CPPUNIT_ASSERT( command.Execute("xxxx") == -1);
//  CPPUNIT_ASSERT( command.GetError());
//  CPPUNIT_ASSERT( command.GetOutput().empty());
}

void wxExAppTestFixture::testConfigItem()
{
  std::vector <wxExConfigItem> items;

  // Use specific constructors.
  wxExConfigItem ci_sl("ci-sl", 1, 5, 
    wxEmptyString, CONFIG_SLIDER);
  items.push_back(ci_sl);
  CPPUNIT_ASSERT(ci_sl.GetLabel() == "ci-sl");
  CPPUNIT_ASSERT(ci_sl.GetType() == CONFIG_SLIDER);

  wxExConfigItem ci_vl("ci-vl", 
    CONFIG_STATICLINE, 
    wxEmptyString,
    false,
    wxID_ANY,
    25,
    false,
    -1,
    1);
  items.push_back(ci_vl);
  CPPUNIT_ASSERT(ci_vl.GetType() == CONFIG_STATICLINE);
    
  wxExConfigItem ci_sp("ci-sp", 1, 5);
  items.push_back(ci_sp);
  CPPUNIT_ASSERT(ci_sp.GetLabel() == "ci-sp");
  CPPUNIT_ASSERT(ci_sp.GetType() == CONFIG_SPINCTRL);
  
  wxExConfigItem ci_sp_d("ci-sp-d", 1.0, 5.0,
    wxEmptyString, CONFIG_SPINCTRL_DOUBLE);
  items.push_back(ci_sp_d);
  CPPUNIT_ASSERT(ci_sp_d.GetType() == CONFIG_SPINCTRL_DOUBLE);
  
  wxExConfigItem ci_str("ci-string");
  items.push_back(ci_str);
  CPPUNIT_ASSERT(ci_str.GetType() == CONFIG_STRING);
  
  wxExConfigItem ci_hl("ci-hyper",
    "www.wxwidgets.org", wxEmptyString, 0, CONFIG_HYPERLINKCTRL);
  items.push_back(ci_hl);
  CPPUNIT_ASSERT(ci_hl.GetType() == CONFIG_HYPERLINKCTRL);

  wxExConfigItem ci_st("ci-static",
    "HELLO", wxEmptyString, 0, CONFIG_STATICTEXT);
  items.push_back(ci_st);
  CPPUNIT_ASSERT( ci_st.GetType() == CONFIG_STATICTEXT);
  
  wxExConfigItem ci_int("ci-int", CONFIG_INT);
  items.push_back(ci_int);
  CPPUNIT_ASSERT(ci_int.GetType() == CONFIG_INT);

  std::map<long, const wxString> echoices;
  echoices.insert(std::make_pair(0, "Zero"));
  echoices.insert(std::make_pair(1, "One"));
  echoices.insert(std::make_pair(2, "Two"));
  wxExConfigItem ci_rb("ci-rb", echoices, true);
  items.push_back(ci_rb);
  CPPUNIT_ASSERT(ci_rb.GetType() == CONFIG_RADIOBOX);

  std::map<long, const wxString> cl;
  cl.insert(std::make_pair(0, "Bit One"));
  cl.insert(std::make_pair(1, "Bit Two"));
  cl.insert(std::make_pair(2, "Bit Three"));
  cl.insert(std::make_pair(4, "Bit Four"));
  wxExConfigItem ci_bc("ci-cl", cl, false);
  items.push_back(ci_bc);
  CPPUNIT_ASSERT(ci_bc.GetType() == CONFIG_CHECKLISTBOX);

  std::set<wxString> bchoices;
  bchoices.insert("This");
  bchoices.insert("Or");
  bchoices.insert("Other");
  wxExConfigItem ci_cl_n(bchoices);
  items.push_back(ci_cl_n);
  CPPUNIT_ASSERT(ci_cl_n.GetType() == CONFIG_CHECKLISTBOX_NONAME);
  
  wxExConfigItem ci_user("ci-usr", new wxTextCtrl());
  items.push_back(ci_user);
  CPPUNIT_ASSERT(ci_user.GetType() == CONFIG_USER);

  // Use general constructor, and add all items.
  for (
    int i = CONFIG_ITEM_MIN + 1;
    i < CONFIG_ITEM_MAX;
    i++)
  {
    if (i != CONFIG_USER)
    {
      items.push_back(wxExConfigItem(
        wxString::Format("item%d", i), 
        (wxExConfigType)i));
    }
  }

  // Check members are initialized.
  for (
    auto it = items.begin();
    it != items.end();
    ++it)
  {
    CPPUNIT_ASSERT( it->GetColumns() == -1);
    if (it->GetType() == CONFIG_USER)
      CPPUNIT_ASSERT( it->GetControl() != NULL);
    else 
      CPPUNIT_ASSERT( it->GetControl() == NULL);
    CPPUNIT_ASSERT(!it->GetIsRequired());
    CPPUNIT_ASSERT(!it->GetLabel().empty());
    CPPUNIT_ASSERT( it->GetPage().empty());

    CPPUNIT_ASSERT(
      it->GetType() > CONFIG_ITEM_MIN &&
      it->GetType() < CONFIG_ITEM_MAX);
  }

  wxGridSizer sizer(3);

  // Layout the items and check control is created.
  for (
    auto it = items.begin();
    it != items.end();
    ++it)
  {
    // CONFIG_USER is not yet laid out ok, gives errors.
    if (it->GetType() != CONFIG_USER)
    {
      // Testing on not NULL not possible,
      // not all items need a sizer.
      it->Layout(wxTheApp->GetTopWindow(), &sizer);
    }
    
    CPPUNIT_ASSERT(it->GetControl() != NULL);
  }

  // Now check ToConfig (after Layout).  
  CPPUNIT_ASSERT( ci_str.Layout(wxTheApp->GetTopWindow(), &sizer) != NULL);
  CPPUNIT_ASSERT( ci_st.Layout(wxTheApp->GetTopWindow(), &sizer) == NULL);
  CPPUNIT_ASSERT( ci_str.ToConfig(true));
  CPPUNIT_ASSERT( ci_str.ToConfig(false));
  CPPUNIT_ASSERT(!ci_st.ToConfig(true));
  CPPUNIT_ASSERT(!ci_st.ToConfig(false));
}

void wxExAppTestFixture::testFrame()
{
  wxExFrame* frame = (wxExFrame*)wxTheApp->GetTopWindow();

  std::vector<wxExStatusBarPane> panes;

  panes.push_back(wxExStatusBarPane());

  for (int i = 0; i < 25; i++)
  {
    panes.push_back(wxExStatusBarPane(wxString::Format("Pane%d", i)));
  }
  
  frame->SetupStatusBar(panes);
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
  CPPUNIT_ASSERT(wxExPrinting::Get() != NULL);
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
  CPPUNIT_ASSERT(!header.InfoNeeded());
  
  const wxString str = header.Get(&filename);
  
  CPPUNIT_ASSERT(!str.empty());
  CPPUNIT_ASSERT( str.Contains("hello test"));
  CPPUNIT_ASSERT( str.Contains("AvW"));
  CPPUNIT_ASSERT( str.Contains("Name"));
  CPPUNIT_ASSERT( str.Contains("Purpose"));
  CPPUNIT_ASSERT( str.Contains("Author"));
  CPPUNIT_ASSERT(!str.Contains("Created"));
  CPPUNIT_ASSERT( str.Contains("Copyright"));
  
  wxExFileName newfile("XXXXXXX");
  wxExHeader newheader;
  newheader.Set("hello test", "AvW");
  const wxString newstr = newheader.Get(&newfile);
  
  CPPUNIT_ASSERT(newstr.Contains("Created"));
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

  CPPUNIT_ASSERT( wxExLexers::Get()->GetDefaultStyle().ContainsDefaultStyle());
  CPPUNIT_ASSERT( wxExLexers::Get()->GetDefaultStyle().IsOk());

  CPPUNIT_ASSERT( wxExLexers::Get()->GetFileName().IsOk());

  CPPUNIT_ASSERT(!wxExLexers::Get()->GetMacros().empty());
  CPPUNIT_ASSERT(!wxExLexers::Get()->GetThemeMacros().empty());

  CPPUNIT_ASSERT( wxExLexers::Get()->IndicatorIsLoaded(wxExIndicator(0)));
  CPPUNIT_ASSERT( wxExLexers::Get()->MarkerIsLoaded(wxExMarker(0)));

  CPPUNIT_ASSERT( wxExLexers::Get()->Read());
}

void wxExAppTestFixture::testListView()
{
  wxExListView* listView = new wxExListView(wxTheApp->GetTopWindow());
  
  listView->InsertColumn(wxExColumn("String", wxExColumn::COL_STRING));
  listView->InsertColumn(wxExColumn("Number", wxExColumn::COL_INT));

  CPPUNIT_ASSERT(listView->FindColumn("String")  == 0);
  CPPUNIT_ASSERT(listView->FindColumn("Number") == 1);
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
  
  // See alo testVCS.
}

void wxExAppTestFixture::testNotebook()
{
  wxExNotebook* notebook = new wxExNotebook(wxTheApp->GetTopWindow(), NULL);
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

  std::vector<wxExStatusBarPane> panes;
  panes.push_back(wxExStatusBarPane());
  panes.push_back(wxExStatusBarPane("panex"));
  panes.push_back(wxExStatusBarPane("paney"));
  panes.push_back(wxExStatusBarPane("panez"));

  wxExStatusBar* sb = new wxExStatusBar(frame);
  
  // The next is OK, but asserts in wxWidgets.
  //../src/generic/statusbr.cpp(179): assert "(size_t)n == m_panes.GetCount()" 
  // failed in SetStatusWidths(): status bar field count mismatch
  //../src/common/statbar.cpp(189): assert "(size_t)n == m_panes.GetCount()" 
  // failed in SetStatusStyles(): field number mismatch
  //CPPUNIT_ASSERT(sb->SetFields(panes) == panes.size());
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

// TODO: Add this test that was previously part of vi test
// (and was OK)  
//  CPPUNIT_ASSERT( vi->FindCommand("/", "vi: "));
//  CPPUNIT_ASSERT(!vi->FindCommand("/", "xxx"));
  
  stc->SetText("new text");
  CPPUNIT_ASSERT(stc->GetText() == "new text");
  
  CPPUNIT_ASSERT(stc->SetLexer("cpp"));

  wxExLexer lexer;
  CPPUNIT_ASSERT( lexer.ApplyLexer(wxEmptyString, stc, false));
  CPPUNIT_ASSERT( lexer.ApplyLexer("cpp", stc, false));
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

  stc->AddText("hello");
  CPPUNIT_ASSERT( stc->GetFile().GetContentsChanged());
  stc->GetFile().ResetContentsChanged();
  CPPUNIT_ASSERT(!stc->GetFile().GetContentsChanged());
}
  
void wxExAppTestFixture::testSTCFile()
{
  wxExSTC* stc = new wxExSTC(wxTheApp->GetTopWindow(), wxExFileName(TEST_FILE));
  wxExSTCFile file(stc);

  // The file itself is not assigned.  
  CPPUNIT_ASSERT(!file.GetFileName().GetStat().IsOk());
  CPPUNIT_ASSERT(!file.GetContentsChanged());

  // For more tests see testSTC.
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

  // Sleep a little to allow the event queue for shell to be processed.
  // TODO: Use wxUiActionSimulator for this.
  //wxYield();
  //wxMilliSleep(10);
  //CPPUNIT_ASSERT(shell->GetHistory().Contains("aaa"));

  CPPUNIT_ASSERT(shell->GetPrompt() == ">");
}

void wxExAppTestFixture::testUtil()
{
  CPPUNIT_ASSERT( wxExAlignText("test", "header", true, true,
    wxExLexers::Get()->FindByName("cpp")).size() 
      == wxString("// headertest").size());

  CPPUNIT_ASSERT( wxExClipboardAdd("test"));
  CPPUNIT_ASSERT( wxExClipboardGet() == "test");
  CPPUNIT_ASSERT( wxExGetEndOfText("test", 3).size() == 3);
  CPPUNIT_ASSERT( wxExGetEndOfText("testtest", 3).size() == 3);
  CPPUNIT_ASSERT( wxExGetLineNumber("test on line: 1200") == 1200);
  CPPUNIT_ASSERT( wxExGetNumberOfLines("test\ntest\n") == 3);
  CPPUNIT_ASSERT(!wxExMatchesOneOf(wxFileName("test.txt"), "*.cpp"));
  CPPUNIT_ASSERT( wxExMatchesOneOf(wxFileName("test.txt"), "*.cpp;*.txt"));
  CPPUNIT_ASSERT( wxExSkipWhiteSpace("\n\tt \n    es   t\n") == "t es t");
  CPPUNIT_ASSERT(!wxExTranslate(
    "hello @PAGENUM@ from @PAGESCNT@", 1, 2).Contains("@"));
}

void wxExAppTestFixture::testVCS()
{
  CPPUNIT_ASSERT(wxExVCS::GetCount() > 0);
  
  wxFileName file(TEST_FILE);
  file.Normalize();
  
  wxArrayString ar;
  ar.Add(file.GetFullPath());
  
  // In wxExApp the vcs is Read, so current vcs is known,
  // using this constructor results in command id 0,
  // giving the first command of current vcs, being add.
  wxExVCS vcs(ar);
  
  CPPUNIT_ASSERT( vcs.GetEntry().BuildMenu(100, new wxMenu("test")) > 0);
  CPPUNIT_ASSERT( vcs.DirExists(file));
    
  // There is a problem in wxExecute inside wxExVCS::Execute (it hangs).
//  CPPUNIT_ASSERT( vcs.Execute() != -1);
//  CPPUNIT_ASSERT(!vcs.GetOutput().empty());

  CPPUNIT_ASSERT( vcs.GetEntry().GetCommand().GetCommand() == "add");
  CPPUNIT_ASSERT( vcs.GetFileName().IsOk());
  CPPUNIT_ASSERT(!vcs.GetEntry().GetCommand().IsOpen());
  CPPUNIT_ASSERT( vcs.Read());
  CPPUNIT_ASSERT(!vcs.GetEntry().SupportKeywordExpansion());
  CPPUNIT_ASSERT( vcs.Use());
  
  wxConfigBase::Get()->Write(_("Base folder"), wxGetCwd());
  
  wxExMenu menu;
  CPPUNIT_ASSERT( menu.AppendVCS() );
}

void wxExAppTestFixture::testVCSCommand()
{
  const wxExVCSCommand add("a&dd", 0);
  const wxExVCSCommand blame("blame", 1);
  const wxExVCSCommand co("checkou&t", 2);
  const wxExVCSCommand commit("commit", 3, "main");
  const wxExVCSCommand diff("diff", 4, "popup", "submenu");
  const wxExVCSCommand log("log", 5, "main");
  const wxExVCSCommand help("h&elp", 6, "error", "", "m&e");
  const wxExVCSCommand update("update", 7);
  const wxExVCSCommand none;

  CPPUNIT_ASSERT(add.GetCommand() == "add");
  CPPUNIT_ASSERT(add.GetCommand(true, true) == "a&dd");
  CPPUNIT_ASSERT(help.GetCommand() == "help me");
  CPPUNIT_ASSERT(help.GetCommand(true, true) == "h&elp m&e");
  CPPUNIT_ASSERT(help.GetCommand(false, true) == "h&elp");
  CPPUNIT_ASSERT(help.GetCommand(false, false) == "help");
  
  CPPUNIT_ASSERT(add.GetNo() == 0);
  CPPUNIT_ASSERT(update.GetNo() != add.GetNo());
  CPPUNIT_ASSERT(update.GetNo() != co.GetNo());
  CPPUNIT_ASSERT(update.GetNo() != commit.GetNo());
  CPPUNIT_ASSERT(update.GetNo() != diff.GetNo());
  CPPUNIT_ASSERT(update.GetNo() != help.GetNo());
  CPPUNIT_ASSERT(update.GetNo() != blame.GetNo());
  CPPUNIT_ASSERT(update.GetNo() != none.GetNo());

  CPPUNIT_ASSERT(add.GetType() == wxExVCSCommand::VCS_COMMAND_IS_BOTH);
  CPPUNIT_ASSERT(blame.GetType() == wxExVCSCommand::VCS_COMMAND_IS_BOTH);
  CPPUNIT_ASSERT(commit.GetType() == wxExVCSCommand::VCS_COMMAND_IS_MAIN);
  CPPUNIT_ASSERT(diff.GetType() == wxExVCSCommand::VCS_COMMAND_IS_POPUP);
  CPPUNIT_ASSERT(help.GetType() == wxExVCSCommand::VCS_COMMAND_IS_BOTH);

  CPPUNIT_ASSERT(add.IsAdd());
  CPPUNIT_ASSERT(blame.IsBlame());
  CPPUNIT_ASSERT(co.IsCheckout());
  CPPUNIT_ASSERT(commit.IsCommit());
  CPPUNIT_ASSERT(diff.IsDiff());
  CPPUNIT_ASSERT(help.IsHelp());
  CPPUNIT_ASSERT(log.IsHistory());
  CPPUNIT_ASSERT(blame.IsOpen());
  CPPUNIT_ASSERT(update.IsUpdate());
  CPPUNIT_ASSERT(!help.UseFlags());
  CPPUNIT_ASSERT(help.UseSubcommand());

  CPPUNIT_ASSERT(add.GetSubMenu().empty());
  CPPUNIT_ASSERT(diff.GetSubMenu() == "submenu");
  CPPUNIT_ASSERT(help.GetSubMenu() == "m&e");

  CPPUNIT_ASSERT(none.GetType() == wxExVCSCommand::VCS_COMMAND_IS_NONE);
}

void wxExAppTestFixture::testVCSEntry()
{
  wxExVCSEntry test;
  
  CPPUNIT_ASSERT( test.GetCommand().GetCommand().empty());
  CPPUNIT_ASSERT( test.GetFlags().empty());
  CPPUNIT_ASSERT( test.GetName().empty());
  CPPUNIT_ASSERT( test.GetNo() == -1);
  CPPUNIT_ASSERT( test.GetOutput().empty());
  CPPUNIT_ASSERT(!test.SupportKeywordExpansion());

  // This should have no effect.  
  test.SetCommand(5);
  
  CPPUNIT_ASSERT( test.GetCommand().GetCommand().empty());
  CPPUNIT_ASSERT( test.GetFlags().empty());
  CPPUNIT_ASSERT( test.GetName().empty());
  CPPUNIT_ASSERT( test.GetNo() == -1);
  CPPUNIT_ASSERT( test.GetOutput().empty());
  CPPUNIT_ASSERT(!test.SupportKeywordExpansion());
}

void wxExAppTestFixture::testVi()
{
  wxConfigBase::Get()->Write(_("vi mode"), true);
 
  // Test for modeline support.
  wxExSTC* stc = new wxExSTC(wxTheApp->GetTopWindow(), 
    "// vi: set ts=120 "
    "// this is a modeline");
    
  CPPUNIT_ASSERT(stc->GetTabWidth() == 120);
  
  wxExVi* vi = new wxExVi(stc);
  
  CPPUNIT_ASSERT(!vi->GetIsActive());
  
  vi->Use(true);
  CPPUNIT_ASSERT( vi->GetIsActive());
  
  CPPUNIT_ASSERT( vi->ExecCommand("$"));
  CPPUNIT_ASSERT( vi->ExecCommand("100"));
  CPPUNIT_ASSERT(!vi->ExecCommand("xxx"));
  
  wxKeyEvent event(wxEVT_CHAR);
  event.m_keyCode = 97; // one char 'a'

  // First a enters insert mode, so is handled by vi, not to be skipped.
  CPPUNIT_ASSERT(!vi->OnChar(event));
  wxYield();

  // The next a is to be skipped, we are now in insert mode.
  // TODO: fix test.
  //CPPUNIT_ASSERT( vi->OnChar(event));
  //wxYield();
}
  
wxExAppTestSuite::wxExAppTestSuite()
  : CppUnit::TestSuite("wxExtension test suite")
{
  addTest(new CppUnit::TestCaller<wxExAppTestFixture>(
    "testCommand",
    &wxExAppTestFixture::testCommand));
    
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
