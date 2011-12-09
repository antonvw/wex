////////////////////////////////////////////////////////////////////////////////
// Name:      test.cpp
// Purpose:   Implementation for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <TestCaller.h>
#include <wx/config.h>
//#if wxUSE_UIACTIONSIMULATOR
//  #include "wx/uiaction.h"
//#endif
#include "test.h"

#define TEST_FILE "./test.h"
#define TEST_BIN "./test.bin"

void wxExGuiTestFixture::setUp()
{
  // Create the global lexers object, 
  // it should be present in ~/.wxex-test-app
  // (depending on platform, configuration).
  wxExLexers* lexers = wxExLexers::Get();
}

void  wxExGuiTestFixture::testConfigDialog()
{
  std::vector <wxExConfigItem> items;
  
  wxExConfigItem ci_str("ci-string");
  items.push_back(ci_str);
  
  wxExConfigDialog dlg(wxTheApp->GetTopWindow(), items);
  
  dlg.ForceCheckBoxChecked();
  
  dlg.Reload();
}

void wxExGuiTestFixture::testConfigItem()
{
  std::vector <wxExConfigItem> items;

  // Use specific constructors.
  wxExConfigItem ci_sl("ci-sl", 1, 5, 
    wxEmptyString, CONFIG_SLIDER);
  items.push_back(ci_sl);
  CPPUNIT_ASSERT(ci_sl.GetLabel() == "ci-sl");
  CPPUNIT_ASSERT(ci_sl.GetType() == CONFIG_SLIDER);

  wxExConfigItem ci_vl(wxLI_HORIZONTAL);
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
  
  wxExConfigItem ci_user("ci-usr", new wxTextCtrl(), NULL);
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
      CPPUNIT_ASSERT( it->GetWindow() != NULL);
    else 
      CPPUNIT_ASSERT( it->GetWindow() == NULL);
      
    CPPUNIT_ASSERT(!it->GetIsRequired());
    if (it->GetType() != CONFIG_STATICLINE)
    {
      CPPUNIT_ASSERT(!it->GetLabel().empty());
    }
    
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
    
    CPPUNIT_ASSERT(it->GetWindow() != NULL);
  }

  // Now check ToConfig (after Layout).  
  CPPUNIT_ASSERT( ci_str.Layout(wxTheApp->GetTopWindow(), &sizer) != NULL);
  CPPUNIT_ASSERT( ci_st.Layout(wxTheApp->GetTopWindow(), &sizer) == NULL);
  CPPUNIT_ASSERT( ci_str.ToConfig(true));
  CPPUNIT_ASSERT( ci_str.ToConfig(false));
  CPPUNIT_ASSERT(!ci_st.ToConfig(true));
  CPPUNIT_ASSERT(!ci_st.ToConfig(false));
}

void wxExGuiTestFixture::testDialog()
{
  wxExDialog(wxTheApp->GetTopWindow(), "hello").Show();
}

void wxExGuiTestFixture::testFrame()
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

void wxExGuiTestFixture::testFrd()
{
  CPPUNIT_ASSERT(wxExFindReplaceData::Get() != NULL);
  
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

void wxExGuiTestFixture::testGrid()
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

void wxExGuiTestFixture::testHeader()
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

void wxExGuiTestFixture::testHexMode()
{
  // 0000000000111111111222222222233333333334444444444555555555566666666666
  // 0123456789012345678901234567890123456789012345678901234567890123456789
  // 00000000 30 31 32 33 34 35 36 37  38 39                   0123456789
  wxExSTC* stc = new wxExSTC(
    wxTheApp->GetTopWindow(), "0123456789", wxExSTC::STC_WIN_HEX);
    
  CPPUNIT_ASSERT(stc->GetText() != "0123456789");
  
  stc->SetCurrentPos(5); // 0 <-
  
  wxExHexModeLine hex(stc);
  
  hex.AppendText("0123456789");

  // test the offset field  
  CPPUNIT_ASSERT( hex.IsOffsetField());
  CPPUNIT_ASSERT(!hex.IsHexField());
  CPPUNIT_ASSERT(!hex.IsAsciiField());
  CPPUNIT_ASSERT( hex.IsReadOnly());
  CPPUNIT_ASSERT(!hex.GetInfo().empty());
  CPPUNIT_ASSERT(!hex.Replace('x'));
  CPPUNIT_ASSERT( hex.OtherField() == wxSTC_INVALID_POSITION);
  
  stc->Reload();
  CPPUNIT_ASSERT(stc->GetText() == "01234567890123456789");
  
  // test hex field
  stc->Reload(wxExSTC::STC_WIN_HEX);
  hex.Set(13); // 31 <- (ascii 1)
  CPPUNIT_ASSERT(!hex.IsOffsetField());
  CPPUNIT_ASSERT( hex.IsHexField());
  CPPUNIT_ASSERT(!hex.IsAsciiField());
  CPPUNIT_ASSERT(!hex.IsReadOnly());
  CPPUNIT_ASSERT(!hex.GetInfo().empty());
  CPPUNIT_ASSERT(!hex.Replace('x'));
  CPPUNIT_ASSERT(!hex.Replace('y'));
  CPPUNIT_ASSERT(!hex.Replace('g'));
  CPPUNIT_ASSERT( hex.Replace('a'));
  CPPUNIT_ASSERT( hex.Replace('9'));
  CPPUNIT_ASSERT( hex.Replace('b')); // (ascii ;)
  CPPUNIT_ASSERT( hex.OtherField() != wxSTC_INVALID_POSITION);
  
  stc->Reload();
  CPPUNIT_ASSERT(stc->GetText() == "0;234567890123456789");
  
  // test ascii field
  stc->Reload(wxExSTC::STC_WIN_HEX);
  hex.Set(63); // 5 <-
  CPPUNIT_ASSERT(!hex.IsOffsetField());
  CPPUNIT_ASSERT(!hex.IsHexField());
  CPPUNIT_ASSERT( hex.IsAsciiField());
  CPPUNIT_ASSERT(!hex.IsReadOnly());
  CPPUNIT_ASSERT(!hex.GetInfo().empty());
  CPPUNIT_ASSERT( hex.Replace('x'));
  CPPUNIT_ASSERT( hex.OtherField() != wxSTC_INVALID_POSITION);
  
  stc->Reload();
  CPPUNIT_ASSERT(stc->GetText() == "0;234x67890123456789");
  
  hex.Set(63); // valid
  CPPUNIT_ASSERT( hex.Goto());
  hex.Set(9999); // invalid, should result in goto end
  CPPUNIT_ASSERT( hex.Goto());
}

void wxExGuiTestFixture::testIndicator()
{
  wxExIndicator ind;
  CPPUNIT_ASSERT( !ind.IsOk() );
  
  wxExIndicator indx(5, 2);
  wxExIndicator indy(7, 5);
  
  CPPUNIT_ASSERT( indx.IsOk());
  CPPUNIT_ASSERT( indy.IsOk());
  CPPUNIT_ASSERT( indx < indy );
}

void wxExGuiTestFixture::testLexer()
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

void wxExGuiTestFixture::testLexers()
{
  CPPUNIT_ASSERT(wxExLexers::Get() != NULL);
  
  CPPUNIT_ASSERT( wxExLexers::Get()->ApplyMacro("XXX") == "XXX");
  CPPUNIT_ASSERT( wxExLexers::Get()->ApplyMacro("mark_lcorner") == "10");
  CPPUNIT_ASSERT( wxExLexers::Get()->ApplyMacro("number") == "fore:red");
  CPPUNIT_ASSERT( wxExLexers::Get()->ApplyMacro("number", "asm") == "2");
  CPPUNIT_ASSERT( wxExLexers::Get()->ApplyMacro("number", "cpp") == "4");

  CPPUNIT_ASSERT(!wxExLexers::Get()->BuildWildCards(
    wxFileName(TEST_FILE)).empty());
  CPPUNIT_ASSERT( wxExLexers::Get()->GetCount() > 0);

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

  CPPUNIT_ASSERT(!wxExLexers::Get()->GetMacros("global").empty());
  CPPUNIT_ASSERT(!wxExLexers::Get()->GetMacros("cpp").empty());
  CPPUNIT_ASSERT(!wxExLexers::Get()->GetMacros("pascal").empty());
  CPPUNIT_ASSERT(!wxExLexers::Get()->GetMacros("XXX").empty());
  
  CPPUNIT_ASSERT(!wxExLexers::Get()->GetThemeMacros().empty());

  CPPUNIT_ASSERT(!wxExLexers::Get()->IndicatorIsLoaded(wxExIndicator(0, -1)));
  CPPUNIT_ASSERT( wxExLexers::Get()->MarkerIsLoaded(wxExMarker(0, -1)));

  CPPUNIT_ASSERT( wxExLexers::Get()->Read());
}

void wxExGuiTestFixture::testLink()
{
  wxExSTC* stc = new wxExSTC(
    wxTheApp->GetTopWindow(), 
    "hello stc, Basepath:/usr/bin");
  
  wxExLink link(stc);  
  
  CPPUNIT_ASSERT( link.FindPath("").empty());
  CPPUNIT_ASSERT( link.FindPath("xxxx").empty());
  CPPUNIT_ASSERT(!link.FindPath("./test").empty());
  CPPUNIT_ASSERT( link.FindPath("<test>") == "test");
  CPPUNIT_ASSERT( link.FindPath(":test") == "test");
  CPPUNIT_ASSERT( link.FindPath("c:test") == "c:test");
  CPPUNIT_ASSERT( link.FindPath("c:\\test") == "c:\\test");
  CPPUNIT_ASSERT( link.FindPath("test:50") == "test");
  
  CPPUNIT_ASSERT( link.AddBasePath());
  CPPUNIT_ASSERT(!link.AddBasePath());
  
  CPPUNIT_ASSERT( link.GetPath("test") == "/usr/bin/test");
}

void wxExGuiTestFixture::testListItem()
{
  wxExListViewFileName* listView = new wxExListViewFileName(
    wxTheApp->GetTopWindow(), wxExListViewFileName::LIST_FILE);
  
  wxStopWatch sw;
  sw.Start();

  const int max = 250;
  for (int j = 0; j < max; j++)
  {
    wxExListItem item1(listView, wxExFileName("./test.h"));
    item1.Insert();
    wxExListItem item2(listView, wxExFileName("./test.cpp"));
    item2.Insert();
    wxExListItem item3(listView, wxExFileName("./main.cpp"));
    item3.Insert();
  }
  
  const long add = sw.Time();

  printf("adding %d items took %ld milliseconds\n", 3 * max, add);
  
  sw.Start();
  
  // The File Name column must be translated, otherwise
  // test fails.
  listView->SortColumn(_("File Name"), SORT_ASCENDING);
  
  const long sort = sw.Time();
  
  printf("sorting %d items took %ld milliseconds\n", 3 * max, sort);
    
  CPPUNIT_ASSERT(listView->GetItemText(0, _("File Name")).Contains("main.cpp"));
}
  
void wxExGuiTestFixture::testListView()
{
  wxExListView* listView = new wxExListView(wxTheApp->GetTopWindow());
  
  listView->SetSingleStyle(wxLC_REPORT);
  listView->InsertColumn(wxExColumn("String", wxExColumn::COL_STRING));
  listView->InsertColumn(wxExColumn("Number", wxExColumn::COL_INT));

  CPPUNIT_ASSERT(listView->FindColumn("String") == 0);
  CPPUNIT_ASSERT(listView->FindColumn("Number") == 1);
}

void wxExGuiTestFixture::testManagedFrame()
{
  wxExManagedFrame* frame = (wxExManagedFrame*)wxTheApp->GetTopWindow();

  CPPUNIT_ASSERT(frame->AllowClose(100, NULL));
  
  wxExSTC* stc = new wxExSTC(frame, "hello world");
  wxExVi* vi = &stc->GetVi();
  
  frame->GetViCommand(vi, "/");
  
  CPPUNIT_ASSERT( frame->GetViCommandIsFind());
  CPPUNIT_ASSERT( frame->GetViCommandIsFindNext());
  CPPUNIT_ASSERT(!frame->GetViCommandIsFindPrevious());
}

void wxExGuiTestFixture::testMarker()
{
  wxExMarker marker;
  CPPUNIT_ASSERT( !marker.IsOk() );
  
  wxExMarker markerx(5, 2);
  wxExMarker markery(7, 5);
  
  CPPUNIT_ASSERT( markerx.IsOk());
  CPPUNIT_ASSERT( markery.IsOk());
  CPPUNIT_ASSERT( markerx < markery );
}

void wxExGuiTestFixture::testMenu()
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

void wxExGuiTestFixture::testNotebook()
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

void  wxExGuiTestFixture::testOTL()
{
#if wxExUSE_OTL
  wxExOTL otl;
  
  CPPUNIT_ASSERT( otl.Datasource().empty());
  CPPUNIT_ASSERT(!otl.IsConnected());
  CPPUNIT_ASSERT(!otl.Logoff());
  CPPUNIT_ASSERT( otl.Query("select * from world") == 0);
#endif
}

void  wxExGuiTestFixture::testPrinting()
{
  CPPUNIT_ASSERT(wxExPrinting::Get() != NULL);
  CPPUNIT_ASSERT(wxExPrinting::Get()->GetPrinter() != NULL);
}

void wxExGuiTestFixture::testProcess()
{
  wxExProcess command;
  
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

void wxExGuiTestFixture::testProperty()
{
  wxExProperty inv;
  CPPUNIT_ASSERT( !inv.IsOk() );
  
  wxExProperty prop("man", "ugly");
  
  CPPUNIT_ASSERT( prop.IsOk());
  
  wxExSTC* stc = new wxExSTC(wxTheApp->GetTopWindow(), "hello stc");
  
  prop.Apply(stc);
  CPPUNIT_ASSERT( prop.IsOk());
  
  prop.ApplyReset(stc);
  CPPUNIT_ASSERT( prop.IsOk());
  
  CPPUNIT_ASSERT( prop.GetName() == "man");
}

void wxExGuiTestFixture::testShell()
{
  wxExSTCShell* shell = new wxExSTCShell(wxTheApp->GetTopWindow());
  
  CPPUNIT_ASSERT(shell->GetShellEnabled());
  
  shell->Prompt("test1");
  shell->Prompt("test2");
  shell->Prompt("test3");
  shell->Prompt("test4");

  // Prompting does not add a command to history.
  CPPUNIT_ASSERT(!shell->GetHistory().Contains("test4"));

  // Post 3 'a' chars to the shell, and check whether it comes in the history.
  shell->ProcessChar('a');
  shell->ProcessChar('a');
  shell->ProcessChar('a');
  shell->ProcessChar('\r');

  CPPUNIT_ASSERT(shell->GetHistory().Contains("aaa"));
  CPPUNIT_ASSERT(shell->GetPrompt() == ">");
  CPPUNIT_ASSERT(shell->GetCommand() == "aaa");
  
  // Post 3 'b' chars to the shell, and check whether it comes in the history.
  shell->ProcessChar('b');
  shell->ProcessChar('b');
  shell->ProcessChar('b');
  shell->ProcessChar('\r');

  CPPUNIT_ASSERT(shell->GetHistory().Contains("aaa"));
  CPPUNIT_ASSERT(shell->GetHistory().Contains("bbb"));
  CPPUNIT_ASSERT(shell->GetPrompt() == ">");
  CPPUNIT_ASSERT(shell->GetCommand() == "bbb");
  
  shell->EnableShell(false);
  CPPUNIT_ASSERT(!shell->GetShellEnabled());
  
  const wxString prompt("---------->");
  shell->SetPrompt(prompt);
  CPPUNIT_ASSERT(shell->GetPrompt() == prompt);
  
  shell->Prompt("test1");
  shell->Prompt("test2");
  CPPUNIT_ASSERT(shell->GetPrompt() == prompt);
}

void wxExGuiTestFixture::testStatusBar()
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

void wxExGuiTestFixture::testSTC()
{
  wxExSTC* stc = new wxExSTC(wxTheApp->GetTopWindow(), "hello stc");
  CPPUNIT_ASSERT(stc->GetText() == "hello stc");
  
  stc->AppendText("more text");
  CPPUNIT_ASSERT(stc->GetText() != "hello stc");
  stc->DocumentStart();
  
  CPPUNIT_ASSERT(stc->FindNext("more text", 0));
  
  CPPUNIT_ASSERT(stc->ReplaceAll("more", "less") == 1);
  CPPUNIT_ASSERT(stc->FindNext("less text", 0));

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
  
void wxExGuiTestFixture::testSTCFile()
{
  wxExSTC* stc = new wxExSTC(wxTheApp->GetTopWindow(), wxExFileName(TEST_FILE));
  wxExSTCFile file(stc);

  // The file itself is not assigned.  
  CPPUNIT_ASSERT(!file.GetFileName().GetStat().IsOk());
  CPPUNIT_ASSERT(!file.GetContentsChanged());

  // For more tests see testSTC.
}

void wxExGuiTestFixture::testStyle()
{
  wxExStyle inv;
  CPPUNIT_ASSERT(!inv.IsOk() );
  
  wxExStyle test1("MARK_CIRCLE", "ugly");
  wxExStyle test2("512", "ugly");
  wxExStyle test3("number,string,comment", "fore:blue", "cpp");
  wxExStyle test4("number,string,xxx", "fore:black", "cpp");
  wxExStyle test5("xxx", "fore:black", "cpp");
  
  CPPUNIT_ASSERT(!test1.IsOk());
  CPPUNIT_ASSERT(!test2.IsOk());
  CPPUNIT_ASSERT( test3.IsOk());
  CPPUNIT_ASSERT( test4.IsOk()); // because number, string is ok
  CPPUNIT_ASSERT(!test5.IsOk());
  
  wxExStyle style("mark_circle", "0");
  
  wxExSTC* stc = new wxExSTC(wxTheApp->GetTopWindow(), "hello stc");
  
  style.Apply(stc);
  CPPUNIT_ASSERT( style.IsOk());
  
  CPPUNIT_ASSERT(!style.ContainsDefaultStyle());
}

void wxExGuiTestFixture::testUtil()
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
  CPPUNIT_ASSERT( wxExMatchesOneOf(wxFileName("test.txt"), "*.txt"));
  CPPUNIT_ASSERT( wxExMatchesOneOf(wxFileName("test.txt"), "*.cpp;*.txt"));
  CPPUNIT_ASSERT( wxExSkipWhiteSpace("\n\tt \n    es   t\n") == "t es t");
  CPPUNIT_ASSERT(!wxExTranslate(
    "hello @PAGENUM@ from @PAGESCNT@", 1, 2).Contains("@"));
}

void wxExGuiTestFixture::testVCS()
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

void wxExGuiTestFixture::testVCSCommand()
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

void wxExGuiTestFixture::testVCSEntry()
{
  wxExVCSEntry test;
  
  CPPUNIT_ASSERT( test.GetCommand().GetCommand().empty());
  CPPUNIT_ASSERT( test.GetFlags().empty());
  CPPUNIT_ASSERT( test.GetName().empty());
  CPPUNIT_ASSERT( test.GetOutput().empty());
  CPPUNIT_ASSERT(!test.SupportKeywordExpansion());

  // This should have no effect.  
  test.SetCommand(5);
  
  CPPUNIT_ASSERT( test.GetCommand().GetCommand().empty());
  CPPUNIT_ASSERT( test.GetFlags().empty());
  CPPUNIT_ASSERT( test.GetName().empty());
  CPPUNIT_ASSERT( test.GetOutput().empty());
  CPPUNIT_ASSERT(!test.SupportKeywordExpansion());
}

void wxExGuiTestFixture::testVi()
{
  wxConfigBase::Get()->Write(_("vi mode"), true);
 
  // Test for modeline support.
  wxExSTC* stc = new wxExSTC(wxTheApp->GetTopWindow(), 
    "// vi: set ts=120 "
    "// this is a modeline");
    
  CPPUNIT_ASSERT(stc->GetTabWidth() == 120);
  
  wxExVi* vi = &stc->GetVi();
  
  vi->Use(true);
  CPPUNIT_ASSERT( vi->GetIsActive());
  
  CPPUNIT_ASSERT( vi->ExecCommand("$"));
  CPPUNIT_ASSERT( vi->ExecCommand("100"));
  CPPUNIT_ASSERT(!vi->ExecCommand("xxx"));
  
  CPPUNIT_ASSERT(!vi->GetInsertMode());
  
  // Tried to use  
  //  wxUIActionSimulator sim;
  //  stc->SetFocus();
  //  sim.Char('i');
  //  wxYield();
    
  // First i enters insert mode, so is handled by vi, not to be skipped.
  wxKeyEvent event(wxEVT_CHAR);
  event.m_uniChar = 'i';
  CPPUNIT_ASSERT(!vi->OnChar(event));
  
  CPPUNIT_ASSERT( vi->GetInsertMode());
  
  // Second i (and more) to be handled by stc.
  CPPUNIT_ASSERT( vi->OnChar(event));
  CPPUNIT_ASSERT( vi->OnChar(event));
  CPPUNIT_ASSERT( vi->OnChar(event));
  CPPUNIT_ASSERT( vi->OnChar(event));
  CPPUNIT_ASSERT( vi->OnChar(event));
  CPPUNIT_ASSERT( vi->OnChar(event));
}
  
wxExAppTestSuite::wxExAppTestSuite()
  : CppUnit::TestSuite("wxExtension test suite")
{
  addTest(new CppUnit::TestCaller<wxExGuiTestFixture>(
    "testConfigDialog",
    &wxExGuiTestFixture::testConfigDialog));
    
  addTest(new CppUnit::TestCaller<wxExGuiTestFixture>(
    "testConfigItem",
    &wxExGuiTestFixture::testConfigItem));
    
  addTest(new CppUnit::TestCaller<wxExGuiTestFixture>(
    "testDialog",
    &wxExGuiTestFixture::testDialog));
    
  addTest(new CppUnit::TestCaller<wxExGuiTestFixture>(
    "testFrame",
    &wxExGuiTestFixture::testFrame));

  addTest(new CppUnit::TestCaller<wxExGuiTestFixture>(
    "testFrd",
    &wxExGuiTestFixture::testFrd));
    
  addTest(new CppUnit::TestCaller<wxExGuiTestFixture>(
    "testGrid",
    &wxExGuiTestFixture::testGrid));
    
  addTest(new CppUnit::TestCaller<wxExGuiTestFixture>(
    "testHeader",
    &wxExGuiTestFixture::testHeader));
    
  addTest(new CppUnit::TestCaller<wxExGuiTestFixture>(
    "testHexMode",
    &wxExGuiTestFixture::testHexMode));
    
  addTest(new CppUnit::TestCaller<wxExGuiTestFixture>(
    "testIndicator",
    &wxExGuiTestFixture::testIndicator));
    
  addTest(new CppUnit::TestCaller<wxExGuiTestFixture>(
    "testLexer",
    &wxExGuiTestFixture::testLexer));

  addTest(new CppUnit::TestCaller<wxExGuiTestFixture>(
    "testLexers",
    &wxExGuiTestFixture::testLexers));

  addTest(new CppUnit::TestCaller<wxExGuiTestFixture>(
    "testLink",
    &wxExGuiTestFixture::testLink));
    
  addTest(new CppUnit::TestCaller<wxExGuiTestFixture>(
    "testListItem",
    &wxExGuiTestFixture::testListItem));
    
  addTest(new CppUnit::TestCaller<wxExGuiTestFixture>(
    "testListView",
    &wxExGuiTestFixture::testListView));
    
  addTest(new CppUnit::TestCaller<wxExGuiTestFixture>(
    "testManagedFrame",
    &wxExGuiTestFixture::testManagedFrame));
    
  addTest(new CppUnit::TestCaller<wxExGuiTestFixture>(
    "testMarker",
    &wxExGuiTestFixture::testMarker));
    
  addTest(new CppUnit::TestCaller<wxExGuiTestFixture>(
    "testMenu",
    &wxExGuiTestFixture::testMenu));
    
  addTest(new CppUnit::TestCaller<wxExGuiTestFixture>(
    "testNotebook",
    &wxExGuiTestFixture::testNotebook));
    
  addTest(new CppUnit::TestCaller<wxExGuiTestFixture>(
    "testOTL",
    &wxExGuiTestFixture::testOTL));
    
  addTest(new CppUnit::TestCaller<wxExGuiTestFixture>(
    "testPrinting",
    &wxExGuiTestFixture::testPrinting));
    
  addTest(new CppUnit::TestCaller<wxExGuiTestFixture>(
    "testProcess",
    &wxExGuiTestFixture::testProcess));
    
  addTest(new CppUnit::TestCaller<wxExGuiTestFixture>(
    "testProperty",
    &wxExGuiTestFixture::testProperty));
    
  addTest(new CppUnit::TestCaller<wxExGuiTestFixture>(
    "testShell",
    &wxExGuiTestFixture::testShell));
    
  addTest(new CppUnit::TestCaller<wxExGuiTestFixture>(
    "testStatusBar",
    &wxExGuiTestFixture::testStatusBar));

  addTest(new CppUnit::TestCaller<wxExGuiTestFixture>(
    "testSTC",
    &wxExGuiTestFixture::testSTC));
 
  addTest(new CppUnit::TestCaller<wxExGuiTestFixture>(
    "testSTCFile",
    &wxExGuiTestFixture::testSTCFile));
    
  addTest(new CppUnit::TestCaller<wxExGuiTestFixture>(
    "testStyle",
    &wxExGuiTestFixture::testStyle));
    
  addTest(new CppUnit::TestCaller<wxExGuiTestFixture>(
    "testUtil",
    &wxExGuiTestFixture::testUtil));
    
  addTest(new CppUnit::TestCaller<wxExGuiTestFixture>(
    "testVCS",
    &wxExGuiTestFixture::testVCS));
    
  addTest(new CppUnit::TestCaller<wxExGuiTestFixture>(
    "testVCSCommand",
    &wxExGuiTestFixture::testVCSCommand));
    
  addTest(new CppUnit::TestCaller<wxExGuiTestFixture>(
    "testVCSEntry",
    &wxExGuiTestFixture::testVCSEntry));
    
  addTest(new CppUnit::TestCaller<wxExGuiTestFixture>(
    "testVi",
    &wxExGuiTestFixture::testVi));
}
