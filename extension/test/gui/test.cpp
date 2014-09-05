////////////////////////////////////////////////////////////////////////////////
// Name:      test.cpp
// Purpose:   Implementation for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2014 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/buffer.h>
#include <wx/config.h>
#include <wx/filehistory.h>
#include "test.h"

#define ESC "\x1b"

#define TEST_FILE "./test.h"
#define TEST_BIN "./test.bin"

void wxExGuiTestFixture::setUp()
{
  // Create the global lexers object, 
  // it should be present in ~/.wxex-test-gui
  // (depending on platform, configuration).
  wxExLexers::Get();
  
  wxConfigBase::Get()->Write(_("vi mode"), true);
}

void wxExGuiTestFixture::tearDown() 
{
  wxExTestFixture::tearDown();
}

void wxExGuiTestFixture::testAddress()
{
  wxExSTC* stc = new wxExSTC(wxTheApp->GetTopWindow(), "hello0\nhello1\nhello2\nhello3\nhello4\nhello5");
  const int lines = stc->GetLineCount();
  wxExEx* ex = new wxExEx(stc);
  stc->GotoLineAndSelect(1);
  ex->MarkerAdd('a'); // put marker a on line
  stc->GotoLineAndSelect(2);
  ex->MarkerAdd('b'); // put marker b on line
  
  CPPUNIT_ASSERT( wxExAddress(ex, "").ToLine() == 0);
  CPPUNIT_ASSERT( wxExAddress(ex, "30").ToLine() == lines);
  CPPUNIT_ASSERT( wxExAddress(ex, "40").ToLine() == lines);
  CPPUNIT_ASSERT( wxExAddress(ex, "-40").ToLine() == 1);
  CPPUNIT_ASSERT( wxExAddress(ex, "3-3").ToLine() == 0);
  CPPUNIT_ASSERT( wxExAddress(ex, "3-1").ToLine() == 2);
  CPPUNIT_ASSERT( wxExAddress(ex, ".").ToLine() == 2);
  CPPUNIT_ASSERT( wxExAddress(ex, ".+1").ToLine() == 3);
  CPPUNIT_ASSERT( wxExAddress(ex, "$").ToLine() == lines);
  CPPUNIT_ASSERT( wxExAddress(ex, "$-2").ToLine() == lines - 2);
  CPPUNIT_ASSERT( wxExAddress(ex, "x").ToLine() == 0);
  CPPUNIT_ASSERT( wxExAddress(ex, "'x").ToLine() == 0);
  CPPUNIT_ASSERT( wxExAddress(ex, "1,3s/x/y").ToLine() == 0);
  CPPUNIT_ASSERT( wxExAddress(ex, "'a").ToLine() == 1);
  CPPUNIT_ASSERT( wxExAddress(ex, "'b").ToLine() == 2);
  CPPUNIT_ASSERT( wxExAddress(ex, "'b+10").ToLine() == lines);
  CPPUNIT_ASSERT( wxExAddress(ex, "10+'b").ToLine() == lines);
  CPPUNIT_ASSERT( wxExAddress(ex, "'a+'b").ToLine() == 3);
  CPPUNIT_ASSERT( wxExAddress(ex, "'b+'a").ToLine() == 3);
  CPPUNIT_ASSERT( wxExAddress(ex, "'b-'a").ToLine() == 1);
}

void wxExGuiTestFixture::testAddressRange()
{
  wxExSTC* stc = new wxExSTC(wxTheApp->GetTopWindow(), "hello\nhello1\nhello2");
  wxExEx* ex = new wxExEx(stc);
  stc->GotoLineAndSelect(2);

  // Test valid ranges.
  CPPUNIT_ASSERT( wxExAddressRange(ex).IsOk());
  CPPUNIT_ASSERT( wxExAddressRange(ex, -1).IsOk());
  CPPUNIT_ASSERT( wxExAddressRange(ex, 5).IsOk());
  CPPUNIT_ASSERT( wxExAddressRange(ex, "%").IsOk());
  CPPUNIT_ASSERT( wxExAddressRange(ex, "*").IsOk());
  CPPUNIT_ASSERT( wxExAddressRange(ex, ".").IsOk());
  
  // Test invalid ranges.
  CPPUNIT_ASSERT(!wxExAddressRange(ex, 0).IsOk());
  CPPUNIT_ASSERT(!wxExAddressRange(ex, "0").IsOk());
  CPPUNIT_ASSERT(!wxExAddressRange(ex, "x").IsOk());
  CPPUNIT_ASSERT(!wxExAddressRange(ex, "x,3").IsOk());
  CPPUNIT_ASSERT(!wxExAddressRange(ex, "x,3").Delete());
  CPPUNIT_ASSERT(!wxExAddressRange(ex, "3,x").Filter("ls"));
  CPPUNIT_ASSERT(!wxExAddressRange(ex, "3,x").Indent());
  CPPUNIT_ASSERT(!wxExAddressRange(ex, "3,!").IsOk());
  CPPUNIT_ASSERT(!wxExAddressRange(ex, "3,@").Move(wxExAddress(ex, "2")));
  CPPUNIT_ASSERT(!wxExAddressRange(ex, "1,2").Move(wxExAddress(ex, "x")));
  CPPUNIT_ASSERT(!wxExAddressRange(ex, "1,3").Move(wxExAddress(ex, "2")));
  CPPUNIT_ASSERT(!wxExAddressRange(ex, "3,x").Write("flut"));
  CPPUNIT_ASSERT(!wxExAddressRange(ex, " ,").Yank());
  
  CPPUNIT_ASSERT( wxExAddressRange(ex, "1,3").Delete());
  CPPUNIT_ASSERT( wxExAddressRange(ex, "1,3").Delete());
  
  // Test Substitute and flags.
  CPPUNIT_ASSERT(!wxExAddressRange(ex, "1").Substitute("//y"));
  CPPUNIT_ASSERT(!wxExAddressRange(ex, "0").Substitute("/x/y"));
  CPPUNIT_ASSERT(!wxExAddressRange(ex, "2").Substitute("/x/y/f"));
  CPPUNIT_ASSERT( wxExAddressRange(ex, "1,2").Substitute("/x/y"));
  CPPUNIT_ASSERT( wxExAddressRange(ex, "1,2").Substitute("/x/y/i"));
  CPPUNIT_ASSERT(!wxExAddressRange(ex, "1,2").Substitute("/x/y/f"));
  CPPUNIT_ASSERT( wxExAddressRange(ex, "1,2").Substitute("/x/y/g"));

  // Test implementation.  
  stc->SetText("a\nb\nc\nd\ne\nf\ng\n");
  stc->GotoLine(1);
  CPPUNIT_ASSERT( wxExAddressRange(ex, 5).Delete());
  CPPUNIT_ASSERT( stc->GetLineCount() == 3);
  CPPUNIT_ASSERT(!wxExAddressRange(ex, 0).Delete());
  CPPUNIT_ASSERT( stc->GetLineCount() == 3);
  stc->GotoLine(0);
  CPPUNIT_ASSERT( wxExAddressRange(ex, 2).Yank());
  stc->SelectNone();
  stc->AddText(stc->GetVi().GetMacros().GetRegister('0'));
  CPPUNIT_ASSERT( stc->GetLineCount() == 5);
  CPPUNIT_ASSERT( wxExAddressRange(ex, -2).Delete());
  stc->GotoLine(0);
  CPPUNIT_ASSERT( wxExAddressRange(ex, -2).Delete());
  
  stc->SetText("a\ntiger\ntiger\ntiger\ntiger\nf\ng\n");
  CPPUNIT_ASSERT( stc->GetLineCount() == 8);
  CPPUNIT_ASSERT( wxExAddressRange(ex, "%").Filter("uniq"));
  CPPUNIT_ASSERT( stc->GetLineCount() == 6);
  
  stc->SetText("a\ntiger\ntiger\ntiger\ntiger\nf\ng\n");
  CPPUNIT_ASSERT( stc->GetLineCount() == 8);
  CPPUNIT_ASSERT( wxExAddressRange(ex, "1,2").Move(wxExAddress(ex, "$")));
  CPPUNIT_ASSERT( stc->GetLineCount() == 8);
}

void wxExGuiTestFixture::testConfigDialog()
{
  // Test config dialog using notebook with pages.
  std::vector <wxExConfigItem> items;
  
  wxExConfigItem ci1("string1", "test", "page0");
  items.push_back(ci1);
  wxExConfigItem ci2("string2", "test", "page0");
  items.push_back(ci2);
  
  wxExConfigDialog dlg(wxTheApp->GetTopWindow(), items);
  
  dlg.ForceCheckBoxChecked();
  dlg.Show();
  dlg.Reload();
  
  // Test config dialog without pages.
  std::vector <wxExConfigItem> items2;
  items2.push_back(wxExConfigItem("string1"));
  
  wxExConfigDialog dlg2(wxTheApp->GetTopWindow(), items2);
  dlg2.Show();
  
  // Test config dialog without items.
  wxExConfigDialog dlg3(wxTheApp->GetTopWindow(), std::vector <wxExConfigItem>());
  dlg3.Show();
}

void wxExGuiTestFixture::testConfigItem()
{
  std::vector <wxExConfigItem> items;

  // Use specific constructors.
  wxExConfigItem ci_empty(5);
  items.push_back(ci_empty);
  CPPUNIT_ASSERT(ci_empty.GetType() == CONFIG_EMPTY);
    
  wxExConfigItem ci_sl("ci-sl", 1, 5, 
    wxEmptyString, CONFIG_SLIDER);
  items.push_back(ci_sl);
  CPPUNIT_ASSERT(ci_sl.GetLabel() == "ci-sl");
  CPPUNIT_ASSERT(ci_sl.GetType() == CONFIG_SLIDER);

  wxExConfigItem ci_vl(wxLI_HORIZONTAL, wxEmptyString);
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
  
  wxExConfigItem ci_sp_h("ci-sp-h", 1.0, 5.0,
    wxEmptyString, CONFIG_SPINCTRL_HEX);
  items.push_back(ci_sp_h);
  CPPUNIT_ASSERT(ci_sp_h.GetType() == CONFIG_SPINCTRL_HEX);
  
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
  for (auto& it : items)
  {
    CPPUNIT_ASSERT( it.GetColumns() == -1);
    
    if (it.GetType() == CONFIG_USER)
      CPPUNIT_ASSERT( it.GetWindow() != NULL);
    else 
      CPPUNIT_ASSERT( it.GetWindow() == NULL);
      
    CPPUNIT_ASSERT(!it.GetIsRequired());
    
    if (
      it.GetType() != CONFIG_STATICLINE &&
      it.GetType() != CONFIG_EMPTY)
    {
      CPPUNIT_ASSERT(!it.GetLabel().empty());
    }
    
    CPPUNIT_ASSERT( it.GetPage().empty());

    CPPUNIT_ASSERT(
      it.GetType() > CONFIG_ITEM_MIN &&
      it.GetType() < CONFIG_ITEM_MAX);
  }

  wxGridSizer sizer(3);

  // Layout the items and check control is created.
  for (auto& it : items)
  {
    // CONFIG_USER is not yet laid out ok, gives errors.
    if (it.GetType() != CONFIG_USER)
    {
      // Testing on not NULL not possible,
      // not all items need a sizer.
      it.Layout(wxTheApp->GetTopWindow(), &sizer);
    }
 
    if (it.GetType() != CONFIG_EMPTY)
    {
      CPPUNIT_ASSERT(it.GetWindow() != NULL);
    }
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

void wxExGuiTestFixture::testEx()
{
  wxExSTC* stc = new wxExSTC(wxTheApp->GetTopWindow(), 
    "// vi: set ts=120"); // this is a modeline
    
  CPPUNIT_ASSERT(stc->GetTabWidth() == 120);
  
  wxExEx* ex = new wxExEx(stc);
  
  CPPUNIT_ASSERT( ex->GetIsActive());
  ex->Use(false);
  CPPUNIT_ASSERT(!ex->GetIsActive());
  ex->Use(true);
  CPPUNIT_ASSERT( ex->GetIsActive());
  
  CPPUNIT_ASSERT( ex->GetMacros().GetCount() > 0); // TODO

  // Test last command.  
  CPPUNIT_ASSERT( ex->Command(":.="));
  CPPUNIT_ASSERT( ex->GetLastCommand() == ":set ts=120");
  CPPUNIT_ASSERT(!ex->Command(":xxx"));
  CPPUNIT_ASSERT( ex->GetLastCommand() == ":set ts=120");
  CPPUNIT_ASSERT(!ex->Command(":yyy"));
  CPPUNIT_ASSERT( ex->GetLastCommand() == ":set ts=120");
  CPPUNIT_ASSERT( ex->Command(":10"));
  CPPUNIT_ASSERT( ex->GetLastCommand() == ":10");
  CPPUNIT_ASSERT( ex->Command(":1,$s/this/ok"));
  CPPUNIT_ASSERT( ex->GetLastCommand() == ":1,$s/this/ok");
  CPPUNIT_ASSERT( ex->Command(":g/is/s//ok"));
  CPPUNIT_ASSERT( ex->GetLastCommand() == ":g/is/s//ok");
  CPPUNIT_ASSERT( ex->Command(":g/is/d"));
  CPPUNIT_ASSERT( ex->GetLastCommand() == ":g/is/d");
  CPPUNIT_ASSERT( ex->Command(":g/is/p"));
  CPPUNIT_ASSERT( ex->GetLastCommand() == ":g/is/p");
  CPPUNIT_ASSERT(!ex->Command(":n")); // there is no next
  CPPUNIT_ASSERT( ex->GetLastCommand() == ":g/is/p");
  CPPUNIT_ASSERT(!ex->Command(":prev")); // there is no previous
  CPPUNIT_ASSERT( ex->GetLastCommand() == ":g/is/p");
  CPPUNIT_ASSERT( ex->Command(":!ls -l"));
  CPPUNIT_ASSERT( ex->GetLastCommand() == ":!ls -l");
  CPPUNIT_ASSERT( ex->Command(":1,$s/s/w/"));
  CPPUNIT_ASSERT( ex->GetLastCommand() == ":1,$s/s/w/");
  
  CPPUNIT_ASSERT( ex->Command(":1,$s/$/ZXXX/"));
  CPPUNIT_ASSERT( ex->Command(":1,$s/^/Zxxx/"));
  CPPUNIT_ASSERT( ex->Command(":.s/$/\n"));
  
  CPPUNIT_ASSERT( ex->Command(":1"));
  CPPUNIT_ASSERT( ex->MarkerAdd('t')); // do not use y or w marker, it is a token!!
  CPPUNIT_ASSERT( ex->Command(":$"));
  CPPUNIT_ASSERT( ex->MarkerAdd('u'));
  CPPUNIT_ASSERT( ex->Command(":'t,'us/s/w/"));
  CPPUNIT_ASSERT( ex->GetLastCommand() == ":'t,'us/s/w/");
  
  // Test range.
  stc->SetText("a\nb\nc\nd\ne\nf\ng\nh\ni\nj\nk\n");
  CPPUNIT_ASSERT( ex->Command(":1,2>"));
  stc->SelectNone();
  CPPUNIT_ASSERT(!ex->Command(":'<,'>>"));
  stc->GotoLine(2);
  stc->LineDownExtend();
  CPPUNIT_ASSERT( ex->Command(":'<,'>m1"));
  stc->GotoLine(2);
  stc->LineDownExtend();
  CPPUNIT_ASSERT( ex->Command(":'<,'>wtest-ex.txt"));
  CPPUNIT_ASSERT( ex->Command(":'<,'><"));
  CPPUNIT_ASSERT( ex->Command(":'<,'>>"));
  CPPUNIT_ASSERT( ex->Command(":'<,'>!sort"));
  stc->GotoLine(2);
  stc->LineDownExtend();
  CPPUNIT_ASSERT( ex->Command(":'<,'>x")); // gives unknown range command: ok
  
  // Test set.
  CPPUNIT_ASSERT( ex->Command(":set ic"));
  CPPUNIT_ASSERT( ex->Command(":set ic!"));
  CPPUNIT_ASSERT( ex->Command(":set li"));
  CPPUNIT_ASSERT( ex->Command(":set li!"));
  CPPUNIT_ASSERT( ex->Command(":set nu"));
  CPPUNIT_ASSERT( ex->Command(":set nu!"));
  CPPUNIT_ASSERT( ex->Command(":set ts=10"));
  CPPUNIT_ASSERT( ex->Command(":set tabstop=10"));
  CPPUNIT_ASSERT(!ex->Command(":set xxx"));
  
  CPPUNIT_ASSERT( ex->Command(":d"));
  //CPPUNIT_ASSERT( ex->Command(":e")); // shows dialog
  CPPUNIT_ASSERT(!ex->Command(":n"));
  CPPUNIT_ASSERT(!ex->Command(":prev"));
  CPPUNIT_ASSERT( ex->Command(":r test"));
  CPPUNIT_ASSERT( ex->Command(":r !echo qwerty"));
  CPPUNIT_ASSERT( stc->GetText().Contains("qwerty"));
  CPPUNIT_ASSERT( ex->Command(":y"));
  CPPUNIT_ASSERT( ex->Command(":1,$s/^/BEGIN-OF-LINE"));
  CPPUNIT_ASSERT( ex->Command(":w test-ex.txt"));

  // Test macros.
  // Do not load macros yet, to test IsRecorded.
  CPPUNIT_ASSERT(!ex->GetMacros().IsRecording());
  CPPUNIT_ASSERT(!ex->GetMacros().IsRecorded("a"));
  
  ex->MacroStartRecording("a");
  CPPUNIT_ASSERT( ex->GetMacros().IsRecording());
  CPPUNIT_ASSERT(!ex->GetMacros().IsRecorded("a"));
  
  ex->GetMacros().StopRecording();
  CPPUNIT_ASSERT(!ex->GetMacros().IsRecording());
  CPPUNIT_ASSERT(!ex->GetMacros().IsRecorded("a")); // still no macro
  
  ex->MacroStartRecording("a");
  CPPUNIT_ASSERT( ex->Command(":10"));
  ex->GetMacros().StopRecording();
  
  CPPUNIT_ASSERT(!ex->GetMacros().IsRecording());
  CPPUNIT_ASSERT( ex->GetMacros().IsRecorded("a"));
  CPPUNIT_ASSERT( ex->GetMacros().StartsWith("a"));
  CPPUNIT_ASSERT(!ex->GetMacros().StartsWith("b"));
  
  CPPUNIT_ASSERT(!ex->GetMacros().IsRecorded("b"));
  
  CPPUNIT_ASSERT( ex->MacroPlayback("a"));
//  CPPUNIT_ASSERT(!ex->MacroPlayback("b"));
  CPPUNIT_ASSERT( ex->GetMacros().GetMacro() == "a");
  CPPUNIT_ASSERT( ex->GetSTC() == stc);

  CPPUNIT_ASSERT( wxExViMacros::LoadDocument());
  CPPUNIT_ASSERT(!ex->GetMacros().IsRecorded("xxx"));
  CPPUNIT_ASSERT( ex->GetMacros().GetCount() > 0);
  
  CPPUNIT_ASSERT( ex->GetMacros().StartsWith("Da"));
  
  CPPUNIT_ASSERT( ex->GetMacros().Expand(ex, "Date"));
//  CPPUNIT_ASSERT(!ex->GetMacros().Expand(ex, "xxx"));
  
  // Test markers.
  CPPUNIT_ASSERT( ex->MarkerAdd('a'));
  CPPUNIT_ASSERT( ex->MarkerLine('a') != -1);
  CPPUNIT_ASSERT( ex->MarkerGoto('a'));
  CPPUNIT_ASSERT( ex->MarkerDelete('a'));
  CPPUNIT_ASSERT(!ex->MarkerDelete('b'));
  CPPUNIT_ASSERT(!ex->MarkerGoto('a'));
  CPPUNIT_ASSERT(!ex->MarkerDelete('a'));
  
  // Test global delete (previous delete was on ont found text).
  stc->AppendText("line xxxx 1 added\n");
  stc->AppendText("line xxxx 2 added\n");
  stc->AppendText("line xxxx 3 added\n");
  stc->AppendText("line xxxx 4 added\n");
  stc->AppendText("line xxxx 5 added\n");
  stc->AppendText("line xxxx 6 added\n");
  
  const int lines = stc->GetLineCount();
  CPPUNIT_ASSERT( ex->Command(":g/xxxx/d"));
  CPPUNIT_ASSERT( stc->GetLineCount() == lines - 6);
  
  // Test global print.
  stc->AppendText("line xxxx 3 added\n");
  stc->AppendText("line xxxx 4 added\n");
  CPPUNIT_ASSERT( ex->Command(":g/xxxx/p"));
  
  // Test global substitute.
  stc->AppendText("line xxxx 6 added\n");
  stc->AppendText("line xxxx 7 added\n");
  CPPUNIT_ASSERT( ex->Command(":g/xxxx/s//yyyy"));
  CPPUNIT_ASSERT( stc->GetText().Contains("yyyy"));
  CPPUNIT_ASSERT( ex->Command(":g//"));
  
  // Test goto.
  stc->SetText("a\nb\nc\nd\ne\nf\ng\nh\ni\nj\nk\n");
  CPPUNIT_ASSERT( stc->GetLineCount() == 12);
  stc->GotoLine(2);
  CPPUNIT_ASSERT( ex->Command(":1"));
  CPPUNIT_ASSERT( stc->GetCurrentLine() == 0);
  CPPUNIT_ASSERT( ex->Command(":-10"));
  CPPUNIT_ASSERT( stc->GetCurrentLine() == 0);
  CPPUNIT_ASSERT( ex->Command(":10"));
  CPPUNIT_ASSERT( stc->GetCurrentLine() == 9);
  CPPUNIT_ASSERT( ex->Command(":10000"));
  CPPUNIT_ASSERT( stc->GetCurrentLine() == 11);
  
  ex->SetRegistersDelete("x");
  ex->SetRegisterYank("test");
  CPPUNIT_ASSERT( ex->GetMacros().GetRegister('0') == "test");
  CPPUNIT_ASSERT( ex->GetRegisterText() == "test");
  
  stc->SetText("the chances");
  stc->SelectAll();
  ex->Yank();
  CPPUNIT_ASSERT( ex->GetRegisterText() == "the chances");
  ex->Cut();
  CPPUNIT_ASSERT( ex->GetRegisterText() == "the chances");
  CPPUNIT_ASSERT( ex->GetMacros().GetRegister('1') == "the chances");
  CPPUNIT_ASSERT( ex->GetSelectedText().empty());
}

void wxExGuiTestFixture::testFileDialog()
{
  wxExFile file;
  wxExFileDialog dlg(wxTheApp->GetTopWindow(), &file);
  
  CPPUNIT_ASSERT(dlg.ShowModalIfChanged(false) == wxID_OK);
}

void wxExGuiTestFixture::testFileStatistics()
{
  wxExFileStatistics fileStatistics;
  
  CPPUNIT_ASSERT(fileStatistics.Get().empty());
  CPPUNIT_ASSERT(fileStatistics.Get("xx") == 0);

  wxExFileStatistics fileStatistics2;
  CPPUNIT_ASSERT(fileStatistics2.Get().empty());

  fileStatistics += fileStatistics2;
  
  CPPUNIT_ASSERT(fileStatistics.Get().empty());
}

void wxExGuiTestFixture::testFrame()
{
  wxExFrame* frame = (wxExFrame*)wxTheApp->GetTopWindow();
  frame->SetFocus(); // otherwise focus is on stc component caused by testEx

  std::vector<wxExStatusBarPane> panes;

  panes.push_back(wxExStatusBarPane());
  
  for (int i = 0; i < 25; i++)
  {
    panes.push_back(wxExStatusBarPane(wxString::Format("Pane%d", i)));
  }
  
  panes.push_back(wxExStatusBarPane("PaneInfo"));
  panes.push_back(wxExStatusBarPane("PaneLexer"));
  panes.push_back(wxExStatusBarPane("PaneFileType"));
  panes.push_back(wxExStatusBarPane("LastPane"));
  
  wxExStatusBar* sb = frame->SetupStatusBar(panes);
  CPPUNIT_ASSERT( sb != NULL);
  
  CPPUNIT_ASSERT( sb->GetFieldsCount () == panes.size());
  CPPUNIT_ASSERT( sb->SetStatusText("hello", ""));
  CPPUNIT_ASSERT( sb->SetStatusText("hello0", "Pane0"));
  CPPUNIT_ASSERT( sb->SetStatusText("hello1", "Pane1"));
  CPPUNIT_ASSERT( sb->SetStatusText("hello2", "Pane2"));
  CPPUNIT_ASSERT(!sb->SetStatusText("hello3", "Panexxx"));
  CPPUNIT_ASSERT(!sb->SetStatusText("hello25", "Pane25"));
  CPPUNIT_ASSERT( sb->SetStatusText("GoodBye", "LastPane"));
  
  CPPUNIT_ASSERT( sb->GetStatusText("Pane0") == "hello0");
  CPPUNIT_ASSERT( ((wxStatusBar*) sb)->GetStatusText(1) == "hello0");
  CPPUNIT_ASSERT( sb->GetStatusText("Pane1") == "hello1");
  CPPUNIT_ASSERT( sb->GetStatusText("Pane2") == "hello2");
  CPPUNIT_ASSERT( sb->GetStatusText("Panexxx").empty());
  
  CPPUNIT_ASSERT( sb->ShowField("Pane0", false));
  CPPUNIT_ASSERT( ((wxStatusBar*) sb)->GetStatusText(1) == "hello1");
  CPPUNIT_ASSERT(!sb->ShowField("Pane0", false));
  CPPUNIT_ASSERT( ((wxStatusBar*) sb)->GetStatusText(1) == "hello1");
  CPPUNIT_ASSERT( sb->GetStatusText("Pane0").empty());
  CPPUNIT_ASSERT( sb->ShowField("Pane0", true));
  CPPUNIT_ASSERT( ((wxStatusBar*) sb)->GetStatusText(1) == "hello0");
  CPPUNIT_ASSERT( sb->GetStatusText("Pane0") == "hello0");
  CPPUNIT_ASSERT( sb->ShowField("LastPane", false));
  CPPUNIT_ASSERT( sb->GetStatusText("LastPane").empty());
  CPPUNIT_ASSERT(!sb->SetStatusText("BackAgain", "LastPane"));
  CPPUNIT_ASSERT( sb->ShowField("LastPane", true));
  CPPUNIT_ASSERT( sb->GetStatusText("LastPane") == "BackAgain");
  
  CPPUNIT_ASSERT(!frame->OpenFile(wxExFileName(TEST_FILE)));
  CPPUNIT_ASSERT( frame->OpenFile(TEST_FILE, "contents"));
  
  CPPUNIT_ASSERT( frame->GetGrid() == NULL);
  CPPUNIT_ASSERT( frame->GetListView() == NULL);
  CPPUNIT_ASSERT( frame->GetSTC() == NULL);
  
  frame->SetFindFocus(NULL);
  frame->SetFindFocus(frame);
  frame->SetFindFocus(frame->GetSTC());
  
  wxMenuBar* bar = new wxMenuBar();
  frame->SetMenuBar(bar);
  
  frame->StatusBarClicked("test");
  frame->StatusBarClicked("Pane1");
  frame->StatusBarClicked("Pane2");
  
  frame->StatusBarClickedRight("test");
  frame->StatusBarClickedRight("Pane1");
  frame->StatusBarClickedRight("Pane2");
  
  CPPUNIT_ASSERT(!frame->StatusText("hello", "test"));
  CPPUNIT_ASSERT( frame->StatusText("hello1", "Pane1"));
  CPPUNIT_ASSERT( frame->StatusText("hello2", "Pane2"));
  CPPUNIT_ASSERT( frame->GetStatusText("Pane1") = "hello1");
  CPPUNIT_ASSERT( frame->GetStatusText("Pane2") = "hello2");
  
  CPPUNIT_ASSERT(!frame->UpdateStatusBar(frame->GetSTC(), "test"));
  CPPUNIT_ASSERT(!frame->UpdateStatusBar(frame->GetSTC(), "Pane1"));
  CPPUNIT_ASSERT(!frame->UpdateStatusBar(frame->GetSTC(), "Pane2"));
  CPPUNIT_ASSERT(!frame->UpdateStatusBar(frame->GetSTC(), "PaneInfo"));
  
  wxExSTC* stc = new wxExSTC(wxTheApp->GetTopWindow(), "hello stc");
  stc->SetFocus();
  
  CPPUNIT_ASSERT( frame->GetSTC() == stc);
  CPPUNIT_ASSERT( frame->UpdateStatusBar(stc, "PaneInfo"));
  CPPUNIT_ASSERT( frame->UpdateStatusBar(stc, "PaneLexer"));
  CPPUNIT_ASSERT( frame->UpdateStatusBar(stc, "PaneFileType"));
}

void wxExGuiTestFixture::testFrd()
{
  CPPUNIT_ASSERT(wxExFindReplaceData::Get() != NULL);
  
  wxExFindReplaceData* frd = wxExFindReplaceData::Get(); 
  
  frd->SetMatchCase(true);
  CPPUNIT_ASSERT( frd->MatchCase());
  frd->SetMatchWord(true);
  CPPUNIT_ASSERT( frd->MatchWord());
  frd->SetUseRegularExpression(true);
  CPPUNIT_ASSERT( frd->UseRegularExpression());

  frd->SetFindString("find1");
  frd->SetFindString("find2");
  frd->SetFindString("find[0-9]");
  CPPUNIT_ASSERT(!frd->GetFindStrings().empty());
  CPPUNIT_ASSERT( frd->GetFindString() == "find[0-9]");
  CPPUNIT_ASSERT( frd->GetRegularExpression().IsValid());
  CPPUNIT_ASSERT( frd->GetRegularExpression().Matches("find9"));

  std::list < wxString > l;
  l.push_back("find3");
  l.push_back("find4");
  l.push_back("find5");
  frd->SetFindStrings(l);
  CPPUNIT_ASSERT( frd->GetFindString() == "find3");
  
  frd->SetReplaceString("replace1");
  frd->SetReplaceString("replace2");
  frd->SetReplaceString("replace[0-9]");
  CPPUNIT_ASSERT(!frd->GetReplaceStrings().empty());
  CPPUNIT_ASSERT( frd->GetReplaceString() == "replace[0-9]");
  
  frd->SetReplaceStrings(l);
  CPPUNIT_ASSERT( frd->GetFindString() == "find3");
  CPPUNIT_ASSERT( frd->GetReplaceString() == "find3");
  
  CPPUNIT_ASSERT( frd->Set(frd->GetTextMatchCase(), false));
  CPPUNIT_ASSERT(!frd->MatchCase());
  CPPUNIT_ASSERT( frd->Set(frd->GetTextMatchWholeWord(), false));
  CPPUNIT_ASSERT(!frd->MatchWord());
  CPPUNIT_ASSERT(!frd->Set("XXXX", false));
}

void wxExGuiTestFixture::testGrid()
{
  wxExGrid* grid = new wxExGrid(wxTheApp->GetTopWindow());
  
  CPPUNIT_ASSERT(grid->CreateGrid(5, 5));
  
  grid->SetGridCellValue(wxGridCellCoords(0, 0), "test");
  
  grid->GoToCell(0, 0);
  CPPUNIT_ASSERT( grid->GetSelectedCellsValue().empty());
  CPPUNIT_ASSERT( grid->GetCellValue(0, 0) == "test");
  
  grid->SetCellsValue(wxGridCellCoords(0, 0), "test1\ttest2\ntest3\ttest4\n");
  CPPUNIT_ASSERT(grid->GetCellValue(0, 0) == "test1");
  
  grid->ClearSelection();
  grid->EmptySelection();
  grid->SetFocus();
  
  CPPUNIT_ASSERT( grid->FindNext("test1"));
  CPPUNIT_ASSERT(!grid->FindNext("text1"));
  
  CPPUNIT_ASSERT(grid->CopySelectedCellsToClipboard());
  
//  grid->Print();
  grid->PrintPreview();
#if wxUSE_DRAG_AND_DROP
  grid->UseDragAndDrop(true);
  grid->UseDragAndDrop(false);
#endif
}

void wxExGuiTestFixture::testHexMode()
{
  // 0000000000111111111222222222233333333334444444444555555555566666666666
  // 0123456789012345678901234567890123456789012345678901234567890123456789
  // 00000000 30 31 32 33 34 35 36 37 38 39                   0123456789
  wxExSTC* stc = new wxExSTC(
    wxTheApp->GetTopWindow(), "0123456789", wxExSTC::STC_WIN_HEX);
    
  CPPUNIT_ASSERT(stc->GetText() != "0123456789");
  
  stc->SetCurrentPos(5); // 0 <-
  
  wxExHexModeLine hex(stc);
  
  hex.AppendText("0123456789");

  // Test the offset field.
  CPPUNIT_ASSERT( hex.IsOffsetField());
  CPPUNIT_ASSERT(!hex.IsHexField());
  CPPUNIT_ASSERT(!hex.IsAsciiField());
  CPPUNIT_ASSERT( hex.IsReadOnly());
  CPPUNIT_ASSERT(!hex.GetInfo().empty());
  CPPUNIT_ASSERT(!hex.Replace('x'));
  CPPUNIT_ASSERT( hex.OtherField() == wxSTC_INVALID_POSITION);

  stc->DiscardEdits();  
  stc->Reload();
  CPPUNIT_ASSERT(stc->GetText() == "01234567890123456789");
  
  // Test hex field.
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
  CPPUNIT_ASSERT( hex.Replace('2'));
  CPPUNIT_ASSERT( hex.OtherField() != wxSTC_INVALID_POSITION);
  
  stc->GetFile().FileSave(wxExFileName("test.hex"));
  stc->Reload();
  CPPUNIT_ASSERT(stc->GetText() == "02234567890123456789");
  
  // Test ascii field.
  stc->Reload(wxExSTC::STC_WIN_HEX);
  hex.Set(63); // 6 <-
  CPPUNIT_ASSERT(!hex.IsOffsetField());
  CPPUNIT_ASSERT(!hex.IsHexField());
  CPPUNIT_ASSERT( hex.IsAsciiField());
  CPPUNIT_ASSERT(!hex.IsReadOnly());
  CPPUNIT_ASSERT(!hex.GetInfo().empty());
  CPPUNIT_ASSERT( hex.Replace('x'));
  CPPUNIT_ASSERT( hex.OtherField() != wxSTC_INVALID_POSITION);
  
  stc->GetFile().FileSave();
  stc->Reload();
  CPPUNIT_ASSERT(stc->GetText() == "022345x7890123456789");
  
  stc->Reload(wxExSTC::STC_WIN_HEX);
  hex.Set(63); // valid
  CPPUNIT_ASSERT( hex.Goto());
  hex.Set(9999); // invalid, should result in goto end
  CPPUNIT_ASSERT( hex.Goto());
  
  // Test hex field.
  stc->Reload(wxExSTC::STC_WIN_HEX);
  hex.Set(13); // 31 <- (ascii 1)
  CPPUNIT_ASSERT( hex.ReplaceHex(32));
  hex.Set(64); // 7 <-
  CPPUNIT_ASSERT(!hex.ReplaceHex(32));
}

void wxExGuiTestFixture::testIndicator()
{
  wxExIndicator ind;
  CPPUNIT_ASSERT(!ind.IsOk() );
  
  wxExIndicator indx(5, 2);
  wxExIndicator indy(7, 5);
  
  CPPUNIT_ASSERT( indx.IsOk());
  CPPUNIT_ASSERT( indy.IsOk());
  CPPUNIT_ASSERT( indx < indy );
}

void wxExGuiTestFixture::testLexer()
{
  wxExSTC* stc = new wxExSTC(wxTheApp->GetTopWindow(), "hello stc");
  
  wxExLexer lexer;
  CPPUNIT_ASSERT(!lexer.IsOk());
  
  CPPUNIT_ASSERT( wxExLexer("cpp").IsOk());
  CPPUNIT_ASSERT( wxExLexer("pascal").IsOk());
  CPPUNIT_ASSERT(!wxExLexer("xxx").IsOk());
  
  lexer = wxExLexers::Get()->FindByText("XXXX");
  CPPUNIT_ASSERT(!lexer.IsOk());
  
  lexer = wxExLexers::Get()->FindByText("<html>");
  CPPUNIT_ASSERT( lexer.IsOk());
  CPPUNIT_ASSERT( lexer.GetDisplayLexer() == "hypertext");
  
  lexer = wxExLexers::Get()->FindByText("// this is a cpp comment text");
  CPPUNIT_ASSERT( lexer.IsOk());
  CPPUNIT_ASSERT( wxExLexer(lexer).IsOk());
  CPPUNIT_ASSERT( lexer.GetDisplayLexer() == "cpp");
  CPPUNIT_ASSERT( lexer.GetScintillaLexer() == "cpp");
  CPPUNIT_ASSERT(!lexer.GetExtensions().empty());
  CPPUNIT_ASSERT(!lexer.GetCommentBegin().empty());
  CPPUNIT_ASSERT(!lexer.GetCommentBegin2().empty());
  CPPUNIT_ASSERT( lexer.GetCommentEnd().empty());
  CPPUNIT_ASSERT(!lexer.GetCommentEnd2().empty());
  CPPUNIT_ASSERT(!lexer.GetKeywords().empty());
  CPPUNIT_ASSERT(!lexer.GetKeywordsString().empty());
  CPPUNIT_ASSERT( lexer.CommentComplete("// test").empty());

  CPPUNIT_ASSERT( lexer.IsKeyword("class"));
  CPPUNIT_ASSERT( lexer.IsKeyword("const"));

  CPPUNIT_ASSERT( lexer.KeywordStartsWith("cla"));
  CPPUNIT_ASSERT(!lexer.KeywordStartsWith("xxx"));

  CPPUNIT_ASSERT(!lexer.MakeComment("test", true).empty());
  CPPUNIT_ASSERT(!lexer.MakeComment("test", "test").empty());
  CPPUNIT_ASSERT(!lexer.MakeSingleLineComment("test").empty());

  CPPUNIT_ASSERT( lexer.GetKeywordsString(6).empty());
  CPPUNIT_ASSERT( lexer.AddKeywords("hello:1"));
  CPPUNIT_ASSERT( lexer.AddKeywords("more:1"));
  CPPUNIT_ASSERT( lexer.AddKeywords(
    "test11 test21:1 test31:1 test12:2 test22:2"));
  CPPUNIT_ASSERT( lexer.AddKeywords("final", 6));
  CPPUNIT_ASSERT(!lexer.AddKeywords(""));
  CPPUNIT_ASSERT(!lexer.AddKeywords("xxx:1", -1));
  CPPUNIT_ASSERT(!lexer.AddKeywords("xxx:1", 100));
  CPPUNIT_ASSERT(!lexer.GetKeywordsString(6).empty());

  CPPUNIT_ASSERT( lexer.IsKeyword("hello")); 
  CPPUNIT_ASSERT( lexer.IsKeyword("more")); 
  CPPUNIT_ASSERT( lexer.IsKeyword("class")); 
  CPPUNIT_ASSERT( lexer.IsKeyword("test11"));
  CPPUNIT_ASSERT( lexer.IsKeyword("test21"));
  CPPUNIT_ASSERT( lexer.IsKeyword("test12"));
  CPPUNIT_ASSERT( lexer.IsKeyword("test22"));
  CPPUNIT_ASSERT( lexer.IsKeyword("test31"));
  CPPUNIT_ASSERT( lexer.IsKeyword("final"));
  CPPUNIT_ASSERT(!lexer.IsKeyword("xxx"));

  CPPUNIT_ASSERT( lexer.KeywordStartsWith("te"));
  CPPUNIT_ASSERT(!lexer.KeywordStartsWith("xx"));

  CPPUNIT_ASSERT(!lexer.GetKeywords().empty());
  
  // TODO: improve test
  lexer.SetProperty("test", "value");
  
  CPPUNIT_ASSERT( lexer.Set("pascal", stc));
  CPPUNIT_ASSERT( lexer.GetDisplayLexer() == "pascal");
  CPPUNIT_ASSERT( lexer.GetScintillaLexer() == "pascal");
  CPPUNIT_ASSERT(!lexer.CommentComplete("(*test").empty());
  CPPUNIT_ASSERT( lexer.CommentComplete("(*test").EndsWith("     *)"));
  
  wxExLexer lexer2(wxExLexers::Get()->FindByText("// this is a cpp comment text"));
  CPPUNIT_ASSERT( lexer2.IsOk());
  CPPUNIT_ASSERT( lexer2.GetDisplayLexer() == "cpp");
  CPPUNIT_ASSERT( lexer2.GetScintillaLexer() == "cpp");
  CPPUNIT_ASSERT( lexer2.Set(lexer, stc));
  CPPUNIT_ASSERT( lexer2.GetDisplayLexer() == "pascal");
  CPPUNIT_ASSERT( lexer2.GetScintillaLexer() == "pascal");
  CPPUNIT_ASSERT(!lexer2.CommentComplete("(*test").empty());
  CPPUNIT_ASSERT( lexer2.CommentComplete("(*test").EndsWith("     *)"));
  
  lexer.Reset(stc);
  CPPUNIT_ASSERT( lexer.GetDisplayLexer().empty());
  CPPUNIT_ASSERT( lexer.GetScintillaLexer().empty());
}

void wxExGuiTestFixture::testLexers()
{
  wxExSTC* stc = new wxExSTC(wxTheApp->GetTopWindow(), "hello stc");
  
  CPPUNIT_ASSERT( wxExLexers::Get() != NULL);
  
  // Test global macro.
  CPPUNIT_ASSERT( wxExLexers::Get()->GetCount() > 0);
  CPPUNIT_ASSERT( wxExLexers::Get()->ApplyMacro("XXX") == "XXX");
  CPPUNIT_ASSERT( wxExLexers::Get()->ApplyMacro("mark_lcorner") == "10");
  CPPUNIT_ASSERT( wxExLexers::Get()->ApplyMacro("mark_circle") == "0");
  CPPUNIT_ASSERT( wxExLexers::Get()->ApplyMacro("iv_none") == "0");
  
  // Test lexer macro.
  CPPUNIT_ASSERT( wxExLexers::Get()->ApplyMacro("number", "asm") == "2");
  CPPUNIT_ASSERT( wxExLexers::Get()->ApplyMacro("number", "cpp") == "4");
  
  CPPUNIT_ASSERT(!wxExLexers::Get()->GetProperties().empty());
  
  wxExLexers::Get()->ApplyMarkers(stc);
  wxExLexers::Get()->ApplyProperties(stc);

  CPPUNIT_ASSERT(!wxExLexers::Get()->BuildWildCards(
    wxFileName(TEST_FILE)).empty());
  CPPUNIT_ASSERT( wxExLexers::Get()->GetCount() > 0);

  CPPUNIT_ASSERT( wxExLexers::Get()->FindByFileName(
    wxFileName(TEST_FILE)).GetScintillaLexer() == "cpp");
    
  CPPUNIT_ASSERT( wxExLexers::Get()->FindByName(
    "cpp").GetScintillaLexer() == "cpp");
    
  CPPUNIT_ASSERT( wxExLexers::Get()->FindByText(
    "// this is a cpp comment text").GetScintillaLexer() == "cpp");
    
  CPPUNIT_ASSERT( wxExLexers::Get()->FindByText(
    "#!/bin/sh").GetScintillaLexer() == "bash");
    
  CPPUNIT_ASSERT( wxExLexers::Get()->FindByText(
    "#!/bin/sh").GetDisplayLexer() == "sh");
    
  CPPUNIT_ASSERT( wxExLexers::Get()->FindByText(
    "#!/bin/csh").GetScintillaLexer() == "bash");
    
  CPPUNIT_ASSERT( wxExLexers::Get()->FindByText(
    "#!/bin/csh").GetDisplayLexer() == "csh");
    
  CPPUNIT_ASSERT( wxExLexers::Get()->FindByText(
    "#!/bin/tcsh").GetScintillaLexer() == "bash");
    
  CPPUNIT_ASSERT( wxExLexers::Get()->FindByText(
    "#!/bin/tcsh").GetDisplayLexer() == "tcsh");
    
  CPPUNIT_ASSERT( wxExLexers::Get()->FindByText(
    "#!/bin/bash").GetScintillaLexer() == "bash");
    
  CPPUNIT_ASSERT( wxExLexers::Get()->FindByText(
    "#!/usr/bin/csh").GetScintillaLexer() == "bash");
    
  CPPUNIT_ASSERT( wxExLexers::Get()->FindByText(
    "<html>").GetScintillaLexer() == "hypertext");
    
  CPPUNIT_ASSERT( wxExLexers::Get()->FindByText(
    "<?xml").GetScintillaLexer() == "hypertext");
    
  CPPUNIT_ASSERT( wxExLexers::Get()->FindByName(
    "xxx").GetScintillaLexer().empty());
    
  CPPUNIT_ASSERT( wxExLexers::Get()->GetCount() > 0);

  CPPUNIT_ASSERT( wxExLexers::Get()->GetDefaultStyle().ContainsDefaultStyle());
  CPPUNIT_ASSERT( wxExLexers::Get()->GetDefaultStyle().IsOk());

  CPPUNIT_ASSERT( wxExLexers::Get()->GetFileName().IsOk());

  CPPUNIT_ASSERT(!wxExLexers::Get()->GetMacros("global").empty());
  CPPUNIT_ASSERT(!wxExLexers::Get()->GetMacros("cpp").empty());
  CPPUNIT_ASSERT(!wxExLexers::Get()->GetMacros("pascal").empty());
  CPPUNIT_ASSERT( wxExLexers::Get()->GetMacros("XXX").empty());
  
  CPPUNIT_ASSERT(!wxExLexers::Get()->GetTheme().empty());
  CPPUNIT_ASSERT( wxExLexers::Get()->GetThemeOk());
  CPPUNIT_ASSERT(!wxExLexers::Get()->GetThemeMacros().empty());

  CPPUNIT_ASSERT(!wxExLexers::Get()->SetTheme("xxx"));
  CPPUNIT_ASSERT(!wxExLexers::Get()->GetTheme().empty());
  CPPUNIT_ASSERT( wxExLexers::Get()->GetThemeOk());
  CPPUNIT_ASSERT( wxExLexers::Get()->SetTheme("torte"));
  CPPUNIT_ASSERT( wxExLexers::Get()->GetTheme() == "torte");
  CPPUNIT_ASSERT( wxExLexers::Get()->GetThemeOk());
  wxExLexers::Get()->SetThemeNone();
  CPPUNIT_ASSERT( wxExLexers::Get()->GetTheme().empty());
  CPPUNIT_ASSERT(!wxExLexers::Get()->GetThemeOk());
  wxExLexers::Get()->RestoreTheme();
  CPPUNIT_ASSERT( wxExLexers::Get()->GetTheme() == "torte");
  CPPUNIT_ASSERT( wxExLexers::Get()->GetThemeOk());
  
  CPPUNIT_ASSERT(!wxExLexers::Get()->IndicatorIsLoaded(wxExIndicator(99, -1)));
  CPPUNIT_ASSERT( wxExLexers::Get()->IndicatorIsLoaded(wxExIndicator(0, -1)));
  CPPUNIT_ASSERT( wxExLexers::Get()->MarkerIsLoaded(wxExMarker(0, -1)));
  
  wxString lexer("cpp");
  wxExLexers::Get()->ShowDialog(wxTheApp->GetTopWindow(), lexer, wxEmptyString, false);
  wxExLexers::Get()->ShowThemeDialog(wxTheApp->GetTopWindow(), wxEmptyString, false);
  
  CPPUNIT_ASSERT(!wxExLexers::Get()->GetKeywords("cpp").empty());
  CPPUNIT_ASSERT(!wxExLexers::Get()->GetKeywords("csh").empty());
  CPPUNIT_ASSERT( wxExLexers::Get()->GetKeywords("xxx").empty());
  CPPUNIT_ASSERT( wxExLexers::Get()->GetKeywords(wxEmptyString).empty());

  CPPUNIT_ASSERT( wxExLexers::Get()->LoadDocument());
}

void wxExGuiTestFixture::link(
  const wxExLink& link,
  const wxString& path, 
  const wxString& expect,
  int expect_line_no,
  int expect_col_no)
{
  int line_no = 0;
  int col_no = 0;

#ifdef DEBUG  
  wxLogMessage("%s %s %d %d\n", 
    path.c_str(), expect.c_str(), 
    expect_line_no, expect_col_no);
#endif

  if (!expect.empty())
  {
    CPPUNIT_ASSERT(link.GetPath(path, line_no, col_no).Contains(expect));
  }
  else
  {
    CPPUNIT_ASSERT(link.GetPath(path, line_no, col_no) == expect);
  }
  
  CPPUNIT_ASSERT(line_no == expect_line_no);
  CPPUNIT_ASSERT(col_no == expect_col_no);
}

void wxExGuiTestFixture::testLink()
{
  wxExSTC* stc = new wxExSTC(
    wxTheApp->GetTopWindow(), 
    "hello stc, \"X-Poedit-Basepath: /usr/bin\\n\"");
  
  wxExLink lnk(stc);  
  
  CPPUNIT_ASSERT( lnk.AddBasePath());
  CPPUNIT_ASSERT( lnk.AddBasePath());
  
  // Test empty, or illegal paths.
  link(lnk, "");
  link(lnk, "xxxx");
  link(lnk, "1 othertest:");
  link(lnk, ":test");
  link(lnk, ": xtest");
  link(lnk, "c:test");
  link(lnk, "c:\\test");
  link(lnk, "on xxxx: 1200");
  link(lnk, "on xxxx: not a number");
  
  // Test existing file in current dir.
  link(lnk, "test.h", "/wxExtension/build/test.h");
  link(lnk, "  test.h", "/wxExtension/build/test.h");
  link(lnk, "test-special.h", "/wxExtension/build/test-special.h");
  link(lnk, "  test-special.h", "/wxExtension/build/test-special.h");
  
  // Test output ls -l.
  // -rw-rw-r-- 1 anton anton  2287 nov 17 10:53 test.h
  link(lnk, "-rw-rw-r-- 1 anton anton 35278 nov 24 16:09 test.h", "/wxExtension/build/test.h");

  // Test existing file in the basepath.
  link(lnk, "test", "/usr/bin/test");
  link(lnk, "  test \n", "/usr/bin/test"); // whitespace should be skipped
  link(lnk, "./test", "/usr/bin/./test");
  link(lnk, "<test>", "/usr/bin/test");
  link(lnk, "\"test\"", "/usr/bin/test");
  link(lnk, "skip <test> skip skip", "/usr/bin/test");
  link(lnk, "skip \"test\" skip skip", "/usr/bin/test");
  link(lnk, "skip 'test' skip skip", "/usr/bin/test");
  
  // Test existing file in the basepath, incorrect line and/or col.
  link(lnk, "test:", "/usr/bin/test");
  link(lnk, "test:xyz", "/usr/bin/test");
  link(lnk, "test:50:xyz", "/usr/bin/test", 50);
  link(lnk, "test:abc:xyz", "/usr/bin/test");
  
  // Test existing file, line_no and col no.
  link(lnk, "test:50", "/usr/bin/test", 50);
  link(lnk, "test:50:", "/usr/bin/test", 50);
  link(lnk, "test:50:6", "/usr/bin/test", 50, 6);
  link(lnk, "test:500000", "/usr/bin/test", 500000);
  link(lnk, "test:500000:599", "/usr/bin/test", 500000, 599);
  link(lnk, "skip skip test:50", "/usr/bin/test", 50);
  link(lnk, "test-special.h:10", "/wxExtension/build/test-special.h", 10);
  link(lnk, "test-special.h:10:2", "/wxExtension/build/test-special.h", 10, 2);
  // po file format
  link(lnk, "#: test:120", "/usr/bin/test", 120);
  
  lnk.SetFromConfig();
  
  // Now we have no basepath, so previous test is different.
  link(lnk, "test");
  
  // Test link with default contructor.
  wxExLink lnk2;
  
  CPPUNIT_ASSERT(!lnk2.AddBasePath());
  CPPUNIT_ASSERT(!lnk2.AddBasePath());
  
  link(lnk2, "test");
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

  CPPUNIT_ASSERT(add < 2000);
  
  Report(wxString::Format(
    "wxExListTiem::Insert %d items in %ld ms", 3 * max, add).ToStdString());
  
  sw.Start();
  
  // The File Name column must be translated, otherwise
  // test fails.
  listView->SortColumn(_("File Name"), SORT_ASCENDING);
  
  const long sort = sw.Time();
  
  CPPUNIT_ASSERT(sort < 1000);
  
  Report(wxString::Format(
    "wxExListView::Sort %d items in %ld ms", 3 * max, sort).ToStdString());
    
  CPPUNIT_ASSERT(listView->GetItemText(0, _("File Name")).Contains("main.cpp"));
}
  
void wxExGuiTestFixture::testListView()
{
  wxExListView* listView = new wxExListView(wxTheApp->GetTopWindow());
  
  listView->SetSingleStyle(wxLC_REPORT);
  
  CPPUNIT_ASSERT(listView->AppendColumn(wxExColumn("Int", wxExColumn::COL_INT)) == 0);
  CPPUNIT_ASSERT(listView->AppendColumn(wxExColumn("Date", wxExColumn::COL_DATE)) == 1);
  CPPUNIT_ASSERT(listView->AppendColumn(wxExColumn("Float", wxExColumn::COL_FLOAT)) == 2);
  CPPUNIT_ASSERT(listView->AppendColumn(wxExColumn("String", wxExColumn::COL_STRING)) == 3);

  CPPUNIT_ASSERT(listView->FindColumn("Int") == 0);
  CPPUNIT_ASSERT(listView->FindColumn("Date") == 1);
  CPPUNIT_ASSERT(listView->FindColumn("Float") == 2);
  CPPUNIT_ASSERT(listView->FindColumn("String") == 3);

  listView->InsertItem(0, "test");
  
  CPPUNIT_ASSERT(listView->FindNext("test"));
  
  CPPUNIT_ASSERT(listView->ItemFromText("a new item"));
  CPPUNIT_ASSERT(listView->FindNext("a new item"));
  
  CPPUNIT_ASSERT(listView->ItemToText(0) == "test");
  
  //listView->Print(); // TODO: asserts
  //listView->PrintPreview();

  // Delete all items, to test sorting later on.  
  listView->DeleteAllItems();
  
  listView->ItemsUpdate();
  
  for (int i = 0; i < 10; i++)
  {
    listView->InsertItem(i, wxString::Format("%d", i));
    listView->SetItem(i, 1, wxDateTime::Now().FormatISOCombined(' '));
    listView->SetItem(i, 2, wxString::Format("%f", (float)i / 2.0));
    listView->SetItem(i, 3, wxString::Format("hello %d", i));
  }
  
  // Test sorting.
  CPPUNIT_ASSERT(!listView->SortColumn("xxx"));
  CPPUNIT_ASSERT( listView->SortColumn("Int", SORT_ASCENDING));
  CPPUNIT_ASSERT( listView->GetItemText(0, "Int") == "0");
  CPPUNIT_ASSERT( listView->GetItemText(1, "Int") == "1");
  CPPUNIT_ASSERT( listView->SortColumn("Int", SORT_DESCENDING));
  CPPUNIT_ASSERT( listView->GetItemText(0, "Int") == "9");
  CPPUNIT_ASSERT( listView->GetItemText(1, "Int") == "8");

  CPPUNIT_ASSERT( listView->SortColumn("Date"));
  CPPUNIT_ASSERT( listView->SortColumn("Float"));
  CPPUNIT_ASSERT( listView->SortColumn("String"));
  
  listView->SetItem(0, 1, "incorrect date");
  CPPUNIT_ASSERT(!listView->SortColumn("Date"));
  
  listView->SetItemImage(0, wxART_WARNING);
}

// Also test the toolbar (wxExToolBar).
void wxExGuiTestFixture::testManagedFrame()
{
  wxExManagedFrame* frame = (wxExManagedFrame*)wxTheApp->GetTopWindow();

  CPPUNIT_ASSERT(frame->AllowClose(100, NULL));
  
  wxExSTC* stc = new wxExSTC(frame, "hello world");
  wxExVi* vi = &stc->GetVi();
  
  CPPUNIT_ASSERT( frame->ExecExCommand(ID_EDIT_NEXT) == NULL);
  
  frame->GetExCommand(vi, "/");
  
  frame->HideExBar();
  frame->HideExBar(false);
  
  CPPUNIT_ASSERT(!frame->GetManager().GetPane("VIBAR").IsShown());
  
  frame->ShowExMessage("hello from frame");
  
  frame->SyncAll();
  frame->SyncCloseAll(0);
  
  CPPUNIT_ASSERT( frame->TogglePane("FINDBAR"));
  CPPUNIT_ASSERT( frame->GetManager().GetPane("FINDBAR").IsShown());
  CPPUNIT_ASSERT( frame->TogglePane("OPTIONSBAR"));
  CPPUNIT_ASSERT( frame->GetManager().GetPane("OPTIONSBAR").IsShown());
  CPPUNIT_ASSERT( frame->TogglePane("TOOLBAR"));
  CPPUNIT_ASSERT(!frame->GetManager().GetPane("TOOLBAR").IsShown());
  CPPUNIT_ASSERT( frame->TogglePane("VIBAR"));
  CPPUNIT_ASSERT( frame->GetManager().GetPane("VIBAR").IsShown());
  
  CPPUNIT_ASSERT(!frame->TogglePane("XXXXBAR"));
  CPPUNIT_ASSERT(!frame->GetManager().GetPane("XXXXBAR").IsOk());
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
  
  menu.Append(wxID_SAVE);
  menu.Append(wxID_SAVE, "mysave");
  menu.AppendEdit();
  menu.AppendEdit(true);
  menu.AppendPrint();
  
  wxMenu* submenu = new wxMenu("submenu");
  menu.AppendSubMenu(submenu, "submenu");
  CPPUNIT_ASSERT(menu.AppendTools());
  menu.AppendVCS(wxFileName(), false); // see alo testVCS
}

void wxExGuiTestFixture::testNotebook()
{
  wxExNotebook* notebook = new wxExNotebook(wxTheApp->GetTopWindow(), NULL);
  wxWindow* page1 = new wxWindow(wxTheApp->GetTopWindow(), wxID_ANY);
  wxWindow* page2 = new wxWindow(wxTheApp->GetTopWindow(), wxID_ANY);
  wxWindow* page3 = new wxWindow(wxTheApp->GetTopWindow(), wxID_ANY);
  wxWindow* page4 = new wxWindow(wxTheApp->GetTopWindow(), wxID_ANY);
  wxWindow* page5 = new wxWindow(wxTheApp->GetTopWindow(), wxID_ANY);
  
  // Test AddPage. 
  CPPUNIT_ASSERT(notebook->AddPage(page1, "key1") != NULL);
  CPPUNIT_ASSERT(notebook->AddPage(page2, "key2") != NULL);
  CPPUNIT_ASSERT(notebook->AddPage(page3, "key3") != NULL);
  // pages: 0,1,2 keys: key1, key2, key3 pages page1,page2,page3.
  
  // Test GetKeyByPage, GetPageByKey, GetPageIndexByKey.
  CPPUNIT_ASSERT(notebook->GetKeyByPage(page1) == "key1");
  CPPUNIT_ASSERT(notebook->GetPageByKey("key1") == page1);
  CPPUNIT_ASSERT(notebook->GetPageIndexByKey("key1") == 0);
  CPPUNIT_ASSERT(notebook->GetPageIndexByKey("xxx") == wxNOT_FOUND);
  
  // Test SetPageText.
  CPPUNIT_ASSERT(notebook->SetPageText("key1", "keyx", "hello"));
  CPPUNIT_ASSERT(notebook->GetPageByKey("keyx") == page1);
  // pages: 0,1,2 keys: keyx, key2, key3 pages page1, page2,page3.
  CPPUNIT_ASSERT(notebook->GetPageIndexByKey("key1") == wxNOT_FOUND);
  
  // Test DeletePage.
  CPPUNIT_ASSERT(notebook->DeletePage("keyx"));
  CPPUNIT_ASSERT(notebook->GetPageByKey("keyx") == NULL);
  CPPUNIT_ASSERT(notebook->DeletePage("key2"));
  CPPUNIT_ASSERT(!notebook->DeletePage("xxx"));
  // pages: 0 keys: key3 pages:page3.

  // Test InsertPage.
  CPPUNIT_ASSERT(notebook->InsertPage(0, page4, "KEY1") != NULL);
  CPPUNIT_ASSERT(notebook->InsertPage(0, page5, "KEY0") != NULL);
  // pages: 0,1,2 keys: KEY0, KEY1, key3 pages: page5,page4,page3.
  CPPUNIT_ASSERT(notebook->GetPageIndexByKey("KEY0") == 0);
  CPPUNIT_ASSERT(notebook->GetPageIndexByKey("KEY1") == 1);
  
  // Test SetSelection.
  CPPUNIT_ASSERT(notebook->SetSelection("KEY1") == page4);
  CPPUNIT_ASSERT(notebook->SetSelection("key3") == page3);
  CPPUNIT_ASSERT(notebook->SetSelection("XXX") == NULL);
  
  // Prepare next test, delete all pages.
  CPPUNIT_ASSERT(notebook->DeletePage("KEY0"));
  CPPUNIT_ASSERT(notebook->DeletePage("KEY1"));
  CPPUNIT_ASSERT(notebook->DeletePage("key3"));
  CPPUNIT_ASSERT(notebook->GetPageCount() == 0);
  
  // Test ForEach. ForEach expects wxExSTC pages.  
  wxExSTC* stc_x = new wxExSTC(wxTheApp->GetTopWindow(), "hello stc");
  wxExSTC* stc_y = new wxExSTC(wxTheApp->GetTopWindow(), "hello stc");
  wxExSTC* stc_z = new wxExSTC(wxTheApp->GetTopWindow(), "hello stc");
  
  CPPUNIT_ASSERT(notebook->AddPage(stc_x, "key1") != NULL);
  CPPUNIT_ASSERT(notebook->AddPage(stc_y, "key2") != NULL);
  CPPUNIT_ASSERT(notebook->AddPage(stc_z, "key3") != NULL);
  
  CPPUNIT_ASSERT(notebook->ForEach(ID_ALL_STC_SET_LEXER));
  CPPUNIT_ASSERT(notebook->ForEach(ID_ALL_STC_SET_LEXER_THEME));
  CPPUNIT_ASSERT(notebook->ForEach(ID_ALL_STC_CONFIG_GET));
  CPPUNIT_ASSERT(notebook->ForEach(ID_ALL_STC_CLOSE_OTHERS));
  CPPUNIT_ASSERT(notebook->GetPageCount() == 1);
  CPPUNIT_ASSERT(notebook->ForEach(ID_ALL_STC_CLOSE));
  CPPUNIT_ASSERT(notebook->GetPageCount() == 0);
}

void wxExGuiTestFixture::testOTL()
{
#if wxExUSE_OTL
  wxExOTL otl;
  
  CPPUNIT_ASSERT( otl.Datasource().empty());
  CPPUNIT_ASSERT(!otl.IsConnected());
  CPPUNIT_ASSERT(!otl.Logoff());
  CPPUNIT_ASSERT( otl.Query("select * from world") == 0);
#endif
}

void wxExGuiTestFixture::testPrinting()
{
  CPPUNIT_ASSERT(wxExPrinting::Get() != NULL);
  CPPUNIT_ASSERT(wxExPrinting::Get()->GetPrinter() != NULL);
  
  wxExSTC* stc = new wxExSTC(wxTheApp->GetTopWindow(), "hello printing");
    
  new wxExPrintout(stc);
}

void wxExGuiTestFixture::testProcess()
{
  wxExProcess process;
  
  CPPUNIT_ASSERT(!process.GetError());
  CPPUNIT_ASSERT( process.GetOutput().empty());
  CPPUNIT_ASSERT(!process.HasStdError());
  CPPUNIT_ASSERT( process.GetShell() == NULL);
  CPPUNIT_ASSERT( process.GetSTC() == NULL);
  CPPUNIT_ASSERT(!process.IsRunning());
  
  process.ConfigDialog(wxTheApp->GetTopWindow(), "test process", false);
  
  // Test wxEXEC_SYNC process.
  /*
  CPPUNIT_ASSERT( process.Execute("ls -l", wxEXEC_SYNC));
  CPPUNIT_ASSERT(!process.GetError());
  CPPUNIT_ASSERT(!process.GetOutput().empty());
  
  CPPUNIT_ASSERT(!process.IsRunning());
  CPPUNIT_ASSERT( process.IsSelected());
  CPPUNIT_ASSERT( process.GetSTC() != NULL);
  CPPUNIT_ASSERT(!process.GetSTC()->GetText().empty());
  CPPUNIT_ASSERT( process.Kill() == wxKILL_NO_PROCESS);
  
  process.ShowOutput();

  // Repeat last wxEXEC_SYNC process.
  // Currently dialog might be cancelled, so do not check return value.
  //  process.Execute("", wxEXEC_SYNC);
  CPPUNIT_ASSERT(!process.GetError());
  CPPUNIT_ASSERT(!process.GetOutput().empty());
*/
  // TODO:
  // Test invalid wxEXEC_SYNC process.
  
  // Test wxEXEC_ASYNC process.
  // wxExecute hangs for wxEXEC_ASYNC
  CPPUNIT_ASSERT( process.Execute("bash"));
  CPPUNIT_ASSERT( process.IsRunning());
  wxExSTCShell* shell = process.GetShell();  
  
  CPPUNIT_ASSERT( shell != NULL);
  
  // Test commands entered in shell.
  const wxString cwd = wxGetCwd();
  
  shell->ProcessChar('c');
  shell->ProcessChar('d');
  shell->ProcessChar(' ');
  shell->ProcessChar('~');
  shell->ProcessChar('\r');
  shell->ProcessChar('p');
  shell->ProcessChar('w');
  shell->ProcessChar('d');
  shell->ProcessChar('\r');
  
  CPPUNIT_ASSERT( shell->GetText().Contains("home"));
//  CPPUNIT_ASSERT( cwd != wxGetCwd());

  // Test invalid wxEXEC_ASYNC process (TODO: but it is started??).
  CPPUNIT_ASSERT( process.Execute("xxxx"));
  CPPUNIT_ASSERT(!process.GetError());
  // The output is not touched by the async process, so if it was not empty,
  // it still is not empty.
//  CPPUNIT_ASSERT(!process.GetOutput().empty());
  
  // TODO:
  // Repeat last process (wxEXEC_ASYNC).
  
  // Go back to where we were, necessary for other tests.
  wxSetWorkingDirectory(cwd);
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
  
  shell->ProcessChar('b');
  shell->ProcessChar('\t'); // tests Expand
  shell->ProcessChar(WXK_BACK);
  shell->ProcessChar(WXK_BACK);
  shell->ProcessChar(WXK_BACK);
  shell->ProcessChar(WXK_BACK);
  shell->ProcessChar(WXK_BACK);
  shell->ProcessChar(WXK_DELETE);
  
  shell->DocumentEnd();
  
  shell->AppendText("hello");
  
  // Test shell enable/disable.
  shell->EnableShell(false);
  CPPUNIT_ASSERT(!shell->GetShellEnabled());
  
  CPPUNIT_ASSERT(!shell->SetPrompt("---------->"));
  CPPUNIT_ASSERT( shell->GetPrompt() == ">");
  
  CPPUNIT_ASSERT(!shell->Prompt("test1"));
  CPPUNIT_ASSERT(!shell->Prompt("test2"));
  CPPUNIT_ASSERT( shell->GetPrompt() == ">");
  
  shell->EnableShell(true);
  CPPUNIT_ASSERT( shell->GetShellEnabled());
  
  shell->Paste();
  
  // Test shell commands.
  shell->SetText("");
  shell->Undo(); // to reset command in shell
  shell->ProcessChar('h');
  shell->ProcessChar('i');
  shell->ProcessChar('s');
  shell->ProcessChar('t');
  shell->ProcessChar('o');
  shell->ProcessChar('r');
  shell->ProcessChar('y');
  shell->ProcessChar('\r');
  CPPUNIT_ASSERT( shell->GetText().Contains("aaa"));
  CPPUNIT_ASSERT( shell->GetText().Contains("bbb"));
  
  shell->SetText("");
  shell->ProcessChar('!');
  shell->ProcessChar('1');
  shell->ProcessChar('\r');
  CPPUNIT_ASSERT( shell->GetText().Contains("aaa"));
  CPPUNIT_ASSERT(!shell->GetText().Contains("bbb"));
  
  shell->SetText("");
  shell->ProcessChar('!');
  shell->ProcessChar('a');
  shell->ProcessChar('\r');
  CPPUNIT_ASSERT( shell->GetText().Contains("aaa"));
  CPPUNIT_ASSERT(!shell->GetText().Contains("bbb"));
}

void wxExGuiTestFixture::testStatusBar()
{
  wxExFrame* frame = (wxExFrame*)wxTheApp->GetTopWindow();
  // Real testing already done in testFrame

  std::vector<wxExStatusBarPane> panes;
  panes.push_back(wxExStatusBarPane());
  panes.push_back(wxExStatusBarPane("panex"));
  panes.push_back(wxExStatusBarPane("paney"));
  panes.push_back(wxExStatusBarPane("panez"));

  //wxExStatusBar* sb = 
  new wxExStatusBar(frame);
}

void wxExGuiTestFixture::testSTC()
{
  // Some methods do not return values, just call them to 
  // prevent cores, and improve test coverage.
  
  wxExSTC::ConfigDialog(wxTheApp->GetTopWindow(), "test stc", wxExSTC::STC_CONFIG_MODELESS);
  
  wxExSTC* stc = new wxExSTC(wxTheApp->GetTopWindow(), "hello stc");
  
  CPPUNIT_ASSERT( stc->GetText() == "hello stc");
  CPPUNIT_ASSERT( stc->FindNext(wxString("hello")));
  
  CPPUNIT_ASSERT(!stc->FindNext(wxString("%d")));
  CPPUNIT_ASSERT(!stc->FindNext(wxString("%ld")));
  CPPUNIT_ASSERT(!stc->FindNext(wxString("%q")));
  
  CPPUNIT_ASSERT( stc->FindNext(wxString("hello"), wxSTC_FIND_WHOLEWORD));
  CPPUNIT_ASSERT(!stc->FindNext(wxString("HELLO"), wxSTC_FIND_MATCHCASE));
  CPPUNIT_ASSERT( stc->GetSearchFlags() & wxSTC_FIND_MATCHCASE);
  // uses flags from frd
  CPPUNIT_ASSERT( stc->FindNext(wxString("HELLO")));
  
  CPPUNIT_ASSERT( stc->AllowChangeIndicator());
  
  stc->AppendText("more text");
  
  CPPUNIT_ASSERT( stc->GetText() != "hello stc");
  
  CPPUNIT_ASSERT( stc->CanCut());
  CPPUNIT_ASSERT( stc->CanPaste());
  
  stc->DocumentStart();
  CPPUNIT_ASSERT( stc->FindNext(wxString("more text")));
  CPPUNIT_ASSERT( stc->GetFindString() == "more text");
  CPPUNIT_ASSERT( stc->ReplaceAll("more", "less") == 1);
  CPPUNIT_ASSERT( stc->ReplaceAll("more", "less") == 0);
  CPPUNIT_ASSERT(!stc->FindNext(wxString("more text")));
  CPPUNIT_ASSERT(!stc->FindNext());
  CPPUNIT_ASSERT( stc->FindNext(wxString("less text")));
  CPPUNIT_ASSERT( stc->ReplaceNext("less text", ""));
  CPPUNIT_ASSERT(!stc->ReplaceNext());
  CPPUNIT_ASSERT(!stc->FindNext(wxString("less text")));
  CPPUNIT_ASSERT( stc->GetFindString() != "less text");
  CPPUNIT_ASSERT( stc->ReplaceAll("%", "percent") == 0);
  
  stc->GotoLineAndSelect(1);
  CPPUNIT_ASSERT(stc->GetCurrentLine() == 0);
  CPPUNIT_ASSERT(stc->GetCurrentPos() == 0);
  stc->GotoLineAndSelect(1, wxEmptyString, 5);
  CPPUNIT_ASSERT(stc->GetCurrentLine() == 0);
  CPPUNIT_ASSERT(stc->GetCurrentPos() == 4);
  
  stc->SetText("new text");
  CPPUNIT_ASSERT(stc->GetText() == "new text");
  
  CPPUNIT_ASSERT(stc->SetLexer("cpp"));

  wxExLexer lexer;
  CPPUNIT_ASSERT( lexer.Reset(stc));
  CPPUNIT_ASSERT( lexer.Set("cpp", stc, false));
  CPPUNIT_ASSERT(!lexer.Set("xyz", stc, false));
  CPPUNIT_ASSERT( stc->SetLexer(lexer));
  
  // do the same test as with wxExFile in base for a binary file
  CPPUNIT_ASSERT(stc->Open(wxExFileName(TEST_BIN)));
  CPPUNIT_ASSERT(stc->GetFlags() == 0);
  const wxCharBuffer& buffer = stc->GetTextRaw();
  CPPUNIT_ASSERT(buffer.length() == 40);

  stc->AddText("hello");
  CPPUNIT_ASSERT( stc->GetFile().GetContentsChanged());
  stc->GetFile().ResetContentsChanged();
  CPPUNIT_ASSERT(!stc->GetFile().GetContentsChanged());
  
  stc->ConfigGet();
  
  stc->Cut();
  
  //  stc->FileTypeMenu();
  
  stc->Fold();
  stc->Fold(true); // FoldAll
  
  CPPUNIT_ASSERT(!stc->GetEOL().empty());
  
  stc->GuessType();
  
  stc->MarkerDeleteAllChange(); // TODO: result
  
  stc->Paste();
  
  CPPUNIT_ASSERT(!stc->PositionRestore());
  stc->PositionSave();
  CPPUNIT_ASSERT( stc->PositionRestore());
  
  //  stc->Print();
  stc->PrintPreview();
  
  stc->ProcessChar(5);
  
  stc->PropertiesMessage();
  
  stc->Reload();
  
  stc->ResetMargins();
  
  stc->SelectNone();
  
  CPPUNIT_ASSERT(!stc->SetIndicator(wxExIndicator(4,5), 100, 200));
  
  stc->SetLexerProperty("xx", "yy");
  
  CPPUNIT_ASSERT(!(stc->GetSearchFlags() & wxSTC_FIND_MATCHCASE));
  wxExFindReplaceData::Get()->SetMatchCase(false);
  stc->SetSearchFlags(-1);
  CPPUNIT_ASSERT(!(stc->GetSearchFlags() & wxSTC_FIND_MATCHCASE));
  
  stc->AutoIndentation('\n');
  
  stc->Sync(false);
  stc->Sync(true);
  
  stc->Undo();
  
  stc->UseModificationMarkers(true);
  stc->UseModificationMarkers(false);
  
  stc->ClearDocument();
  
  stc->Reload(wxExSTC::STC_WIN_HEX);
  CPPUNIT_ASSERT(stc->HexMode());
  stc->AppendTextHexMode("in hex mode");
  
  // Test events.
  wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED);
  
  event.SetInt(ID_EDIT_HEX_DEC_CALLTIP);
  wxPostEvent(stc, event);
  event.SetInt(ID_EDIT_MARKER_NEXT);
  wxPostEvent(stc, event);
  event.SetInt(ID_EDIT_MARKER_PREVIOUS);
  wxPostEvent(stc, event);
  event.SetInt(ID_EDIT_OPEN_LINK);
  wxPostEvent(stc, event);
  event.SetInt(ID_EDIT_SHOW_PROPERTIES);
  wxPostEvent(stc, event);
  event.SetInt(ID_EDIT_ZOOM_IN);
  wxPostEvent(stc, event);
  event.SetInt(ID_EDIT_ZOOM_OUT);
  wxPostEvent(stc, event);
}
  
void wxExGuiTestFixture::testSTCEntryDialog()
{
  wxExSTCEntryDialog dlg1(wxTheApp->GetTopWindow(), "hello", "testing");
  CPPUNIT_ASSERT( dlg1.GetText() == "testing");
  //CPPUNIT_ASSERT( dlg1.GetTextRaw() == "testing");
  CPPUNIT_ASSERT( dlg1.SetLexer("cpp"));
  CPPUNIT_ASSERT(!dlg1.SetLexer("xxx"));
  
  wxExSTCEntryDialog dlg2(
    wxTheApp->GetTopWindow(), 
      "hello", 
      "testing",
      "hello again",
      wxOK,
      true);
  CPPUNIT_ASSERT(!dlg2.GetText().empty());
  CPPUNIT_ASSERT( dlg2.GetTextRaw().length() > 0);
  CPPUNIT_ASSERT( dlg2.GetSTCShell() != NULL);
  CPPUNIT_ASSERT( dlg2.GetSTCShell()->GetPrompt() == "testing");
}

void wxExGuiTestFixture::testSTCFile()
{
  wxExSTC* stc = new wxExSTC(wxTheApp->GetTopWindow(), wxExFileName(TEST_FILE));
  wxExSTCFile file(stc);

  // The file itself is not assigned.  
  CPPUNIT_ASSERT(!file.GetFileName().GetStat().IsOk());
  CPPUNIT_ASSERT(!file.GetContentsChanged());

  CPPUNIT_ASSERT( file.Read("test.bin"));
  
  file.FileNew(wxExFileName("xxxx"));
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

void wxExGuiTestFixture::testTextFile()
{
  // Test find.
  wxExTextFile textFile(wxExFileName(TEST_FILE), ID_TOOL_REPORT_FIND);
  
  wxExFindReplaceData::Get()->SetFindString("test");
  wxExFindReplaceData::Get()->SetMatchCase(true);
  wxExFindReplaceData::Get()->SetMatchWord(true);
  wxExFindReplaceData::Get()->SetUseRegularExpression(false);
  
  wxStopWatch sw;
  sw.Start();
  CPPUNIT_ASSERT( textFile.RunTool());
  const long elapsed = sw.Time();
  
  CPPUNIT_ASSERT(elapsed < 20);
  
  Report(wxString::Format(
    "wxExTextFile::matching %d items in %ld ms", 
    textFile.GetStatistics().Get(_("Actions Completed")), elapsed).ToStdString());
    
  CPPUNIT_ASSERT(!textFile.GetStatistics().GetElements().GetItems().empty());
  CPPUNIT_ASSERT( textFile.GetStatistics().Get(_("Actions Completed")) == 193);
  
  // Test replace.
  wxExTextFile textFile2(wxExFileName(TEST_FILE), ID_TOOL_REPORT_REPLACE);
  
  wxExFindReplaceData::Get()->SetReplaceString("test");
  
  wxStopWatch sw2;
  sw2.Start();
  CPPUNIT_ASSERT( textFile2.RunTool());
  const long elapsed2 = sw2.Time();
  
  CPPUNIT_ASSERT(elapsed2 < 100);
  
  Report(wxString::Format(
    "wxExTextFile::replacing %d items in %ld ms", 
    textFile2.GetStatistics().Get(_("Actions Completed")), elapsed2).ToStdString());
    
  CPPUNIT_ASSERT(!textFile2.GetStatistics().GetElements().GetItems().empty());
  CPPUNIT_ASSERT( textFile2.GetStatistics().Get(_("Actions Completed")) == 194);
}

void wxExGuiTestFixture::testToVectorString()
{
  wxArrayString a;
  a.Add("x");
  a.Add("b");
  a.Add("c");
  a.Add("d");
  wxExToVectorString v1(a);
  CPPUNIT_ASSERT( v1.Get().size() == 4);
  
  wxMenu menu;
  menu.Append(1, "x");
  menu.Append(2, "y");
  wxFileHistory history;
  history.UseMenu(&menu);
  history.AddFileToHistory("test1");
  history.AddFileToHistory("test2");
  wxExToVectorString v2(history, 5);
  CPPUNIT_ASSERT( v2.Get().size() == 2);
  
  wxExToVectorString v3("test test test");
  CPPUNIT_ASSERT( v3.Get().size() == 3);
}

void wxExGuiTestFixture::testUtil()
{
  wxExFrame* frame = (wxExFrame*)wxTheApp->GetTopWindow();
  wxExSTC* stc = new wxExSTC(frame);
  stc->SetFocus();
  
  // wxExAlignText
  CPPUNIT_ASSERT( wxExAlignText("test", "header", true, true,
    wxExLexers::Get()->FindByName("cpp")).size() 
      == wxString("// headertest").size());
      
  // wxExAutoComplete
  wxString s;
  CPPUNIT_ASSERT(!wxExAutoComplete("xxxx", 
    stc->GetVi().GetMacros().Get(), s));
  CPPUNIT_ASSERT(!wxExAutoComplete("Date", // not unique!
    stc->GetVi().GetMacros().Get(), s));
  CPPUNIT_ASSERT( wxExAutoComplete("Datet", 
    stc->GetVi().GetMacros().Get(), s));
  CPPUNIT_ASSERT( s == "Datetime");
  
  // wxExAutoCompleteFileName
  std::vector<wxString> v;
  CPPUNIT_ASSERT( wxExAutoCompleteFileName("te", v));
  
#ifdef DEBUG  
  for (int i = 0; i < v.size(); i++)
  {
    wxLogMessage("v[%d]=%s", i, v[i].c_str());
  }
#endif

  CPPUNIT_ASSERT( v[0] == "st");
  CPPUNIT_ASSERT(!wxExAutoCompleteFileName("XX", v));
  CPPUNIT_ASSERT( v[0] == "st");
  
  // wxExCalculator
  wxExEx* ex = new wxExEx(stc);
  int width = 0;
  CPPUNIT_ASSERT( wxExCalculator("", ex, width) == 0);
  CPPUNIT_ASSERT( width == 0);
  CPPUNIT_ASSERT( wxExCalculator("1 + 1", ex, width) == 2);
  CPPUNIT_ASSERT( width == 0);
  CPPUNIT_ASSERT( wxExCalculator("1 - 1", ex, width) == 0);
  CPPUNIT_ASSERT( width == 0);
  CPPUNIT_ASSERT( wxExCalculator("1 * 1", ex, width) == 1);
  CPPUNIT_ASSERT( width == 0);
  CPPUNIT_ASSERT( wxExCalculator("1,0 + 1", ex, width) == 2);
  CPPUNIT_ASSERT( width == 1);
  CPPUNIT_ASSERT( wxExCalculator("xxx", ex, width) == 0);
  CPPUNIT_ASSERT( width == 0);

  // wxExClipboardAdd
  CPPUNIT_ASSERT( wxExClipboardAdd("test"));
  
  // wxExClipboardGet
  CPPUNIT_ASSERT( wxExClipboardGet() == "test");
  
  // wxExComboBoxFromList
  std::list < wxString > l;
  l.push_back("x");
  l.push_back("y");
  l.push_back("z");
  wxComboBox* cb = new wxComboBox(frame, wxID_ANY);
  wxExComboBoxFromList(cb, l);
  CPPUNIT_ASSERT( cb->GetCount() == 3);
  
  // wxExComboBoxToList
  l.clear();
  l = wxExComboBoxToList(cb);
  CPPUNIT_ASSERT( l.size() == 3);
  
  // wxExCompareFile
  
  // wxExConfigFirstOf
  CPPUNIT_ASSERT( wxExConfigFirstOf("xxxx").empty());
  
  // wxExEllipsed  
  CPPUNIT_ASSERT( wxExEllipsed("xxx").Contains("..."));
  
  // wxExGetEndOfText
  CPPUNIT_ASSERT( wxExGetEndOfText("test", 3).size() == 3);
  CPPUNIT_ASSERT( wxExGetEndOfText("testtest", 3).size() == 3);
  
  // wxExGetFieldSeparator
  CPPUNIT_ASSERT( wxExGetFieldSeparator() != 'a');

  // wxExGetFindResult  
  CPPUNIT_ASSERT( wxExGetFindResult("test", true, true).Contains("test"));
  CPPUNIT_ASSERT( wxExGetFindResult("test", true, false).Contains("test"));
  CPPUNIT_ASSERT( wxExGetFindResult("test", false, true).Contains("test"));
  CPPUNIT_ASSERT( wxExGetFindResult("test", false, false).Contains("test"));
  
  CPPUNIT_ASSERT( wxExGetFindResult("%d", true, true).Contains("%d"));
  CPPUNIT_ASSERT( wxExGetFindResult("%d", true, false).Contains("%d"));
  CPPUNIT_ASSERT( wxExGetFindResult("%d", false, true).Contains("%d"));
  CPPUNIT_ASSERT( wxExGetFindResult("%d", false, false).Contains("%d"));
  
  // wxExGetHexNumberFromUser
  
  // wxExGetIconID
  CPPUNIT_ASSERT( wxExGetIconID( wxFileName(TEST_FILE)) != -1);

  // wxExGetNumberOfLines  
  CPPUNIT_ASSERT( wxExGetNumberOfLines("test") == 1);
  CPPUNIT_ASSERT( wxExGetNumberOfLines("test\n") == 2);
  CPPUNIT_ASSERT( wxExGetNumberOfLines("test\ntest") == 2);
  CPPUNIT_ASSERT( wxExGetNumberOfLines("test\ntest\n") == 3);
  CPPUNIT_ASSERT( wxExGetNumberOfLines("test\rtest\r") == 3);
  CPPUNIT_ASSERT( wxExGetNumberOfLines("test\r\ntest\n") == 3);
  
  CPPUNIT_ASSERT( wxExGetNumberOfLines("test\r\ntest\n\n\n", true) == 2);
  CPPUNIT_ASSERT( wxExGetNumberOfLines("test\r\ntest\n\n", true) == 2);
  CPPUNIT_ASSERT( wxExGetNumberOfLines("test\r\ntest\n\n", true) == 2);
  
  // wxExGetWord
  
  // wxExIsBrace
  CPPUNIT_ASSERT( wxExIsBrace('('));
  CPPUNIT_ASSERT( wxExIsBrace(')'));
  CPPUNIT_ASSERT( wxExIsBrace('{'));
  CPPUNIT_ASSERT(!wxExIsBrace('a'));
  
  // wxExIsCodewordSeparator
  CPPUNIT_ASSERT( wxExIsCodewordSeparator('('));
  CPPUNIT_ASSERT( wxExIsCodewordSeparator(','));
  CPPUNIT_ASSERT(!wxExIsCodewordSeparator('x'));

  // wxExListFromConfig
  CPPUNIT_ASSERT( wxExListFromConfig("xxx").size() == 0);
  
  // wxExListToConfig
  l.clear();
  l.push_back("1");
  l.push_back("2");
  wxExListToConfig(l, "list_items");
  CPPUNIT_ASSERT( l.size() == 2); // TODO: improve, test on config
  
  // wxExLogStatus
  wxExLogStatus( wxExFileName(TEST_FILE));

  // wxExMake  
  CPPUNIT_ASSERT( wxExMake(wxFileName("xxx")) != -1);
  CPPUNIT_ASSERT( wxExMake(wxFileName("make.tst")) != -1);

  // wxExMatch
  CPPUNIT_ASSERT( wxExMatch("([0-9]+)ok([0-9]+)nice", "19999ok245nice", v) == 2);
  CPPUNIT_ASSERT( wxExMatch("(\\d+)ok(\\d+)nice", "19999ok245nice", v) == 2);
  CPPUNIT_ASSERT( wxExMatch(" ([\\d\\w]+)", " 19999ok245nice ", v) == 1);
  
  // wxExMatchesOneOf
  CPPUNIT_ASSERT(!wxExMatchesOneOf(wxFileName("test.txt"), "*.cpp"));
  CPPUNIT_ASSERT( wxExMatchesOneOf(wxFileName("test.txt"), "*.txt"));
  CPPUNIT_ASSERT( wxExMatchesOneOf(wxFileName("test.txt"), "*.cpp;*.txt"));
  
  // wxExNodeProperties
  
  // wxExNodeStyles
  
  // wxExOpenFiles
  std::vector<wxString> files;
  wxExOpenFiles(frame, files);
  wxFileName file(TEST_FILE);
  file.Normalize();
  files.push_back(file.GetFullPath());
  files.push_back("test.cpp");
  files.push_back("*xxxxxx*.cpp");
  wxExOpenFiles(frame, files);
  
  // wxExOpenFilesDialog
  
  // wxExPrintCaption
  CPPUNIT_ASSERT( wxExPrintCaption(wxFileName("test")).Contains("test"));
  
  // wxExPrintFooter
  CPPUNIT_ASSERT( wxExPrintFooter().Contains("@"));
  
  // wxExPrintHeader
  CPPUNIT_ASSERT( wxExPrintHeader(wxFileName(TEST_FILE)).Contains("test"));
  
  // wxExQuoted
  CPPUNIT_ASSERT( wxExQuoted("test") == "'test'");
  CPPUNIT_ASSERT( wxExQuoted("%d") == "'%d'");
  CPPUNIT_ASSERT( wxExQuoted(wxExSkipWhiteSpace(wxString(" %d "))) == "'%d'");
  
  // wxExSetTextCtrlValue

  // wxExSkipWhiteSpace
  CPPUNIT_ASSERT( wxExSkipWhiteSpace("\n\tt \n    es   t\n") == "t es t");
  
  // wxExTranslate
  CPPUNIT_ASSERT(!wxExTranslate(
    "hello @PAGENUM@ from @PAGESCNT@", 1, 2).Contains("@"));
    
  // wxExVCSCommandOnSTC
  wxExVCSCommand command("status");
  wxExVCSCommandOnSTC(command, wxExLexer("cpp"), stc);
  
  // wxExVCSExecute
  // wxExVCSExecute(frame, 0, files); // calls dialog
  
  delete ex;
}

void wxExGuiTestFixture::testVariable()
{
  wxExSTC* stc = new wxExSTC(wxTheApp->GetTopWindow(), "hello again");
  wxExEx* ex = new wxExEx(stc);
  
  wxExVariable v;
  CPPUNIT_ASSERT( v.Expand(ex));
  CPPUNIT_ASSERT( v.GetName().empty());
  CPPUNIT_ASSERT(!v.IsModified());
  
  wxXmlNode xml(wxXML_ELEMENT_NODE, "variable");
  xml.AddAttribute("name", "test");
  xml.AddAttribute("type", "BUILTIN");
    
  wxExVariable var(&xml);
  CPPUNIT_ASSERT( var.GetName() == "test");
  CPPUNIT_ASSERT( var.GetValue().IsEmpty());
  CPPUNIT_ASSERT(!var.Expand(ex));
  CPPUNIT_ASSERT(!var.IsModified());
  
  xml.DeleteAttribute("name");
  xml.AddAttribute("name", "Year");
  
  wxExVariable var2(&xml);
  CPPUNIT_ASSERT( var2.GetName() == "Year");
  CPPUNIT_ASSERT( var2.Expand(ex));
  CPPUNIT_ASSERT(!var2.IsModified());
  
  wxExVariable var3("added");
  CPPUNIT_ASSERT( var3.GetName() == "added");
  // This is input, we cannot test it at this moment.
}

void wxExGuiTestFixture::testVCS()
{
  CPPUNIT_ASSERT(wxExVCS::GetCount() > 0);
  
  wxFileName file(TEST_FILE);
  file.Normalize();
  
  std::vector< wxString > v;
  v.push_back(file.GetFullPath());
  
  // In wxExApp the vcs is Read, so current vcs is known,
  // using this constructor results in command id 0,
  // giving the first command of current vcs, being add.
  wxExVCS vcs(v);
  
  vcs.ConfigDialog(wxTheApp->GetTopWindow(), "test vcs", false);
  
  CPPUNIT_ASSERT( vcs.GetCount() > 0);
  CPPUNIT_ASSERT( vcs.GetEntry().BuildMenu(100, new wxMenu("test")) > 0);
  CPPUNIT_ASSERT( vcs.DirExists(file));
    
  // We do not have a vcs bin, so execute fails.
// TODO: next crashes due to select file dialog.
//  CPPUNIT_ASSERT( vcs.Execute() == -1);
  CPPUNIT_ASSERT( vcs.GetEntry().GetOutput().empty());

  CPPUNIT_ASSERT( vcs.GetEntry().GetCommand().GetCommand() == "add");
  CPPUNIT_ASSERT( vcs.GetFileName().IsOk());
  CPPUNIT_ASSERT(!vcs.GetEntry().GetCommand().IsOpen());
  CPPUNIT_ASSERT( wxExVCS::LoadDocument());
  CPPUNIT_ASSERT( vcs.Use());
  
  wxConfigBase::Get()->Write(_("Base folder"), wxGetCwd());
  
  wxExMenu menu;
  CPPUNIT_ASSERT( menu.AppendVCS(wxFileName(), false) );
}

void wxExGuiTestFixture::testVCSCommand()
{
  const wxExVCSCommand add("a&dd");
  const wxExVCSCommand blame("blame");
  const wxExVCSCommand co("checkou&t");
  const wxExVCSCommand commit("commit", "main");
  const wxExVCSCommand diff("diff", "popup", "submenu");
  const wxExVCSCommand log("log", "main");
  const wxExVCSCommand help("h&elp", "error", "", "m&e");
  const wxExVCSCommand update("update");
  const wxExVCSCommand none;

  CPPUNIT_ASSERT(add.GetCommand() == "add");
  CPPUNIT_ASSERT(add.GetCommand(true, true) == "a&dd");
  CPPUNIT_ASSERT(help.GetCommand() == "help me");
  CPPUNIT_ASSERT(help.GetCommand(true, true) == "h&elp m&e");
  CPPUNIT_ASSERT(help.GetCommand(false, true) == "h&elp");
  CPPUNIT_ASSERT(help.GetCommand(false, false) == "help");
  
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
  
  CPPUNIT_ASSERT( test.GetCommands() == 1);
  
  wxExVCSEntry test2;
  
  CPPUNIT_ASSERT( test2.GetCommands() == 1);
  
  CPPUNIT_ASSERT( test.GetCommand().GetCommand().empty());
  CPPUNIT_ASSERT(!test.AdminDirIsTopLevel());
  CPPUNIT_ASSERT( test.GetFlags().empty());
  CPPUNIT_ASSERT( test.GetName().empty());
  CPPUNIT_ASSERT( test.GetOutput().empty());
  
  CPPUNIT_ASSERT( test.ShowDialog(
    wxTheApp->GetTopWindow(),
    "vcs",
    false) == wxID_CANCEL);
    
  test.ShowOutput();
  
  wxMenu menu;
  CPPUNIT_ASSERT( test.BuildMenu(0, &menu) == 0);
  
  // This should have no effect.  
  CPPUNIT_ASSERT(!test.SetCommand(5));
  CPPUNIT_ASSERT(!test.SetCommand(ID_EDIT_VCS_LOWEST));
  CPPUNIT_ASSERT(!test.SetCommand(ID_VCS_LOWEST));
  
  CPPUNIT_ASSERT( test.GetCommand().GetCommand().empty());
  CPPUNIT_ASSERT( test.GetFlags().empty());
  CPPUNIT_ASSERT( test.GetName().empty());
  CPPUNIT_ASSERT( test.GetOutput().empty());
}

void wxExGuiTestFixture::testVersion()
{
  CPPUNIT_ASSERT(!wxExVersionInfo().GetVersionOnlyString().empty());
  CPPUNIT_ASSERT(!wxExGetVersionInfo().GetVersionOnlyString().empty());
}

void wxExGuiTestFixture::testVi()
{
  const int esc = 27;
 
  // Test for modeline support.
  wxExSTC* stc = new wxExSTC(wxTheApp->GetTopWindow(), 
    "// vi: set ts=120 "
    "// this is a modeline");
    
  CPPUNIT_ASSERT(stc->GetTabWidth() == 120);
  
  wxExVi* vi = &stc->GetVi();
  
  CPPUNIT_ASSERT( vi->GetIsActive());
  
  // Repeat some ex tests.
  CPPUNIT_ASSERT( vi->Command(":$"));
  CPPUNIT_ASSERT( vi->Command(":100"));
  CPPUNIT_ASSERT(!vi->Command(":xxx"));
  
  CPPUNIT_ASSERT( vi->GetMode() == wxExVi::MODE_NORMAL);
  
  wxKeyEvent event(wxEVT_CHAR);
  
  // Test WXK_NONE.
  event.m_uniChar = WXK_NONE;
  CPPUNIT_ASSERT( vi->OnChar(event));
  
  // First i enters insert mode, so is handled by vi, not to be skipped.
  event.m_uniChar = 'i';
  CPPUNIT_ASSERT(!vi->OnChar(event));
  CPPUNIT_ASSERT( vi->GetMode() == wxExVi::MODE_INSERT);
  
  // Second i (and more) all handled by vi.
  CPPUNIT_ASSERT(!vi->OnChar(event));
  CPPUNIT_ASSERT(!vi->OnChar(event));
  CPPUNIT_ASSERT(!vi->OnChar(event));
  CPPUNIT_ASSERT(!vi->OnChar(event));
  CPPUNIT_ASSERT(!vi->OnChar(event));
  CPPUNIT_ASSERT(!vi->OnChar(event));

  // Repeat some macro tests.
  CPPUNIT_ASSERT(!vi->GetMacros().IsRecording());
  
  vi->MacroStartRecording("a");
  CPPUNIT_ASSERT( vi->GetMacros().IsRecording());
  CPPUNIT_ASSERT(!vi->GetMacros().IsRecorded("a"));
  
  vi->GetMacros().StopRecording();
  CPPUNIT_ASSERT(!vi->GetMacros().IsRecording());
  CPPUNIT_ASSERT(!vi->GetMacros().IsRecorded("a")); // still no macro
  
  vi->MacroStartRecording("a");
  CPPUNIT_ASSERT(!vi->OnChar(event));
  CPPUNIT_ASSERT( vi->Command(ESC));
  vi->GetMacros().StopRecording();
  CPPUNIT_ASSERT(!vi->GetMacros().IsRecording());
  CPPUNIT_ASSERT( vi->GetMacros().IsRecorded("a"));
  
  CPPUNIT_ASSERT(!vi->GetMacros().IsRecorded("b"));
  
  CPPUNIT_ASSERT( vi->MacroPlayback("a"));
//  CPPUNIT_ASSERT(!vi->MacroPlayback("b"));

  // Be sure we are in normal mode.
  CPPUNIT_ASSERT( vi->Command(ESC));
  CPPUNIT_ASSERT( vi->GetMode() == wxExVi::MODE_NORMAL);
  
  // Vi control key tests.
  event.m_uniChar = WXK_CONTROL_B;
  CPPUNIT_ASSERT( vi->OnKeyDown(event));
  CPPUNIT_ASSERT(!vi->OnChar(event));
  event.m_uniChar = WXK_CONTROL_E;
  CPPUNIT_ASSERT( vi->OnKeyDown(event));
  CPPUNIT_ASSERT(!vi->OnChar(event));
  event.m_uniChar = WXK_CONTROL_F;
  CPPUNIT_ASSERT( vi->OnKeyDown(event));
  CPPUNIT_ASSERT(!vi->OnChar(event));
  event.m_uniChar = WXK_CONTROL_G;
  CPPUNIT_ASSERT( vi->OnKeyDown(event));
  CPPUNIT_ASSERT(!vi->OnChar(event));
  event.m_uniChar = WXK_CONTROL_J;
  CPPUNIT_ASSERT( vi->OnKeyDown(event));
  CPPUNIT_ASSERT(!vi->OnChar(event));
  event.m_uniChar = WXK_CONTROL_P;
  CPPUNIT_ASSERT( vi->OnKeyDown(event));
  CPPUNIT_ASSERT(!vi->OnChar(event));
  event.m_uniChar = WXK_CONTROL_Q;
  CPPUNIT_ASSERT( vi->OnKeyDown(event));
  CPPUNIT_ASSERT(!vi->OnChar(event));
  
  event.m_keyCode = WXK_BACK;
  CPPUNIT_ASSERT(!vi->OnKeyDown(event));
  event.m_keyCode = WXK_RETURN;
  CPPUNIT_ASSERT(!vi->OnKeyDown(event));
  event.m_keyCode = WXK_TAB;
  CPPUNIT_ASSERT( vi->OnKeyDown(event));
  
  // Vi navigation command tests.
  CPPUNIT_ASSERT( vi->Command(ESC));
  CPPUNIT_ASSERT( vi->GetMode() == wxExVi::MODE_NORMAL);
  
  event.m_keyCode = WXK_LEFT;
  CPPUNIT_ASSERT(!vi->OnKeyDown(event));
  event.m_keyCode = WXK_DOWN;
  CPPUNIT_ASSERT(!vi->OnKeyDown(event));
  event.m_keyCode = WXK_UP;
  CPPUNIT_ASSERT(!vi->OnKeyDown(event));
  event.m_keyCode = WXK_RIGHT;
  CPPUNIT_ASSERT(!vi->OnKeyDown(event));
  event.m_keyCode = WXK_PAGEUP;
  CPPUNIT_ASSERT(!vi->OnKeyDown(event));
  event.m_keyCode = WXK_PAGEDOWN;
  CPPUNIT_ASSERT(!vi->OnKeyDown(event));
  event.m_keyCode = WXK_NONE;
  CPPUNIT_ASSERT( vi->OnKeyDown(event));
  
  // Test navigate with [ and ].
  event.m_uniChar = '[';
  CPPUNIT_ASSERT(!vi->OnChar(event));
  event.m_uniChar= ']';
  CPPUNIT_ASSERT(!vi->OnChar(event));
  vi->GetSTC()->AppendText("{}");
  event.m_uniChar = '[';
  CPPUNIT_ASSERT(!vi->OnChar(event));
  event.m_uniChar= ']';
  CPPUNIT_ASSERT(!vi->OnChar(event));
  
  // Vi command tests.
  stc->SetText("aaaaa");
  CPPUNIT_ASSERT( vi->GetMode() == wxExVi::MODE_NORMAL);
  CPPUNIT_ASSERT( vi->Command("i"));
  CPPUNIT_ASSERT( vi->GetMode() == wxExVi::MODE_INSERT);
  CPPUNIT_ASSERT( vi->Command("xxxxxxxx"));
  CPPUNIT_ASSERT( vi->Command(ESC));
  CPPUNIT_ASSERT( vi->GetMode() == wxExVi::MODE_NORMAL);
  CPPUNIT_ASSERT( stc->GetText().Contains("xxxxxxxx"));
  CPPUNIT_ASSERT( vi->GetInsertText() == "xxxxxxxx");
  CPPUNIT_ASSERT( vi->GetLastCommand() == wxString("ixxxxxxxx") + wxUniChar(esc));
  
  for (int i = 0; i < 10; i++)
    CPPUNIT_ASSERT( vi->Command("."));
    
  CPPUNIT_ASSERT( stc->GetText().Contains("xxxxxxxxxxxxxxxxxxxxxxxxxxx"));
  
  // Test MODE_INSERT commands.
  std::vector<std::string> commands;
  commands.push_back("a");
  commands.push_back("i");
  commands.push_back("o");
  commands.push_back("A");
  commands.push_back("C");
  commands.push_back("I");
  commands.push_back("O");
  commands.push_back("R");
  
  CPPUNIT_ASSERT( vi->GetMode() == wxExVi::MODE_NORMAL);
  
  for (auto& it1 : commands)
  {
    CPPUNIT_ASSERT( vi->Command(it1) );
    CPPUNIT_ASSERT( vi->GetMode() == wxExVi::MODE_INSERT);
    CPPUNIT_ASSERT( vi->Command(ESC));
    CPPUNIT_ASSERT( vi->GetMode() == wxExVi::MODE_NORMAL);
  }
  
  // Test MODE_INSERT commands and delete command on readonly document.
  commands.push_back("dd");
  commands.push_back("d0");
  commands.push_back("d$");
  commands.push_back("dw");
  commands.push_back("de");
  
  stc->SetReadOnly(true);
  stc->EmptyUndoBuffer();
  stc->SetSavePoint();
  
  for (auto& it2 : commands)
  {
    CPPUNIT_ASSERT( vi->Command(it2) );
  }
  
  CPPUNIT_ASSERT( vi->GetMode() == wxExVi::MODE_NORMAL);
  CPPUNIT_ASSERT(!stc->GetModify());
  
  // Test MODE_INSERT commands on hexmode document.
  stc->SetReadOnly(false);
  stc->Reload(wxExSTC::STC_WIN_HEX);
  CPPUNIT_ASSERT( stc->HexMode());
  
  for (auto& it3 : commands)
  {
    CPPUNIT_ASSERT( vi->Command(it3) );
  }
  
  CPPUNIT_ASSERT( vi->GetMode() == wxExVi::MODE_NORMAL);
  CPPUNIT_ASSERT(!stc->GetModify());
  
  stc->Reload();
  CPPUNIT_ASSERT(!stc->HexMode());
  
  CPPUNIT_ASSERT(!stc->GetModify());
  stc->SetReadOnly(false);

  // Test insert command.  
  CPPUNIT_ASSERT( vi->Command("i"));
  CPPUNIT_ASSERT( vi->GetMode() == wxExVi::MODE_INSERT);
  CPPUNIT_ASSERT( vi->Command(ESC));
  
  CPPUNIT_ASSERT( vi->Command("iyyyyy"));
  CPPUNIT_ASSERT( vi->GetMode() == wxExVi::MODE_INSERT);
  CPPUNIT_ASSERT( stc->GetText().Contains("yyyyy"));
  CPPUNIT_ASSERT(!stc->GetText().Contains("iyyyyy"));
  CPPUNIT_ASSERT( vi->Command(ESC));
  CPPUNIT_ASSERT( vi->GetMode() == wxExVi::MODE_NORMAL);
  
  const wxString lastcmd = wxString("iyyyyy") + wxUniChar(esc);

  CPPUNIT_ASSERT( vi->GetLastCommand() == lastcmd);
  CPPUNIT_ASSERT( vi->Command("."));
  CPPUNIT_ASSERT( vi->GetLastCommand() == lastcmd);
  CPPUNIT_ASSERT(!vi->Command(";"));
  CPPUNIT_ASSERT( vi->GetLastCommand() == lastcmd);
  CPPUNIT_ASSERT( vi->Command("ma"));
  CPPUNIT_ASSERT( vi->GetLastCommand() == lastcmd);
  
  CPPUNIT_ASSERT( vi->Command("100izz"));
  CPPUNIT_ASSERT( vi->GetMode() == wxExVi::MODE_INSERT);
  CPPUNIT_ASSERT(!stc->GetText().Contains("izz"));
  CPPUNIT_ASSERT( vi->Command(ESC));
  CPPUNIT_ASSERT( stc->GetText().Contains(wxString('z', 200)));
  CPPUNIT_ASSERT( vi->GetMode() == wxExVi::MODE_NORMAL);
  
  stc->SetText("999");
  CPPUNIT_ASSERT( vi->Command("i"));
  CPPUNIT_ASSERT( vi->GetMode() == wxExVi::MODE_INSERT);
  CPPUNIT_ASSERT( vi->Command("b"));
  CPPUNIT_ASSERT( vi->Command(ESC));
  CPPUNIT_ASSERT( stc->GetText().Contains("b"));
  CPPUNIT_ASSERT( vi->GetMode() == wxExVi::MODE_NORMAL);
  
  // Test commands that do not change mode.
  commands.clear();
  commands.push_back("b");
  commands.push_back("e");
  commands.push_back("h");
  commands.push_back("j");
  commands.push_back("k");
  commands.push_back("l");
  commands.push_back(" ");
  commands.push_back("n");
  commands.push_back("p");
  commands.push_back("u");
  commands.push_back("w");
  commands.push_back("x");
//  commands.push_back("y"); // only true if something selected
  commands.push_back("B");
  commands.push_back("D");
  commands.push_back("E");
  commands.push_back("G");
  commands.push_back("H");
  commands.push_back("J");
  commands.push_back("L");
  commands.push_back("M");
  commands.push_back("N");
  commands.push_back("P");
  commands.push_back("W");
  commands.push_back("X");
  commands.push_back("^");
  commands.push_back("~");
  commands.push_back("$");
  commands.push_back("{");
  commands.push_back("}");
  commands.push_back("(");
  commands.push_back(")");
  commands.push_back("%");
  commands.push_back("*");
  commands.push_back("#");
  
  for (auto& it4 : commands)
  {
    CPPUNIT_ASSERT( vi->Command(it4) );
// p changes last command    
//    CPPUNIT_ASSERT( vi.GetLastCommand() == lastcmd);
    CPPUNIT_ASSERT( vi->GetMode() == wxExVi::MODE_NORMAL);
  }

  // Test /, ?, n and N.
  stc->SetText("aaaaa\nbbbbb\nccccc\naaaaa");
  CPPUNIT_ASSERT( vi->Command("/bbbbb"));
  CPPUNIT_ASSERT( stc->GetCurrentLine() == 1);
  CPPUNIT_ASSERT(!vi->Command("/d"));
  CPPUNIT_ASSERT( stc->GetCurrentLine() == 1);
  CPPUNIT_ASSERT( vi->Command("/a"));
  CPPUNIT_ASSERT( stc->GetCurrentLine() == 3);
  CPPUNIT_ASSERT( vi->Command("n"));
  CPPUNIT_ASSERT( stc->GetCurrentLine() == 3);
  CPPUNIT_ASSERT( vi->Command("N"));
  CPPUNIT_ASSERT( stc->GetCurrentLine() == 3);
  stc->SetText("aaaaa\nbbbbb\nccccc\naaaaa");
  CPPUNIT_ASSERT( vi->Command("?bbbbb"));
  CPPUNIT_ASSERT( stc->GetCurrentLine() == 1);
  CPPUNIT_ASSERT(!vi->Command("?d"));
  CPPUNIT_ASSERT( stc->GetCurrentLine() == 1);
  CPPUNIT_ASSERT( vi->Command("?a"));
  CPPUNIT_ASSERT( stc->GetCurrentLine() == 0);
  CPPUNIT_ASSERT( vi->Command("n"));
  CPPUNIT_ASSERT( stc->GetCurrentLine() == 0);
  CPPUNIT_ASSERT( vi->Command("N"));
  CPPUNIT_ASSERT( stc->GetCurrentLine() == 0);
  
  // Test substitute command.
  stc->SetText("xxxxxxxxxx\nxxxxxxxx\naaaaaaaaaa\n");
  CPPUNIT_ASSERT( vi->Command(":.="));
  CPPUNIT_ASSERT( vi->Command(":1,$s/xx/yy/g"));
  CPPUNIT_ASSERT(!stc->GetText().Contains("xx"));
  CPPUNIT_ASSERT( stc->GetText().Contains("yy"));
  CPPUNIT_ASSERT( vi->GetLastCommand() == ":1,$s/xx/yy/g");
  
  // Test change command.
  stc->SetText("xxxxxxxxxx second\nxxxxxxxx\naaaaaaaaaa\n");
  CPPUNIT_ASSERT( stc->GetLineCount() == 4);
  CPPUNIT_ASSERT( vi->Command(":1"));
  CPPUNIT_ASSERT( vi->Command("cc"));
  CPPUNIT_ASSERT( vi->GetMode() == wxExVi::MODE_INSERT);
  CPPUNIT_ASSERT( vi->Command("zzz"));
  CPPUNIT_ASSERT( vi->Command(ESC));
  CPPUNIT_ASSERT( vi->GetMode() == wxExVi::MODE_NORMAL);
  CPPUNIT_ASSERT( stc->GetLineCount() == 4);
  CPPUNIT_ASSERT( stc->GetLineText(0) == "zzz");
  
  stc->SetText("xxxxxxxxxx second\nxxxxxxxx\naaaaaaaaaa\n");
  CPPUNIT_ASSERT( vi->Command(":1"));
  CPPUNIT_ASSERT( vi->Command("cw"));
  CPPUNIT_ASSERT( vi->GetMode() == wxExVi::MODE_INSERT);
  CPPUNIT_ASSERT( vi->Command("zzz"));
  CPPUNIT_ASSERT( vi->Command(ESC));
  CPPUNIT_ASSERT( vi->GetMode() == wxExVi::MODE_NORMAL);
  CPPUNIT_ASSERT( stc->GetLineCount() == 4);
  CPPUNIT_ASSERT( stc->GetLineText(0) == "zzz second");
  
  // Test delete command.
  CPPUNIT_ASSERT( vi->Command("dw"));
  CPPUNIT_ASSERT( vi->Command("3dw"));
  CPPUNIT_ASSERT( vi->GetLastCommand() == "3dw");
  CPPUNIT_ASSERT( vi->Command("dd"));
  CPPUNIT_ASSERT( vi->Command("de"));
  CPPUNIT_ASSERT( vi->Command("d0"));
  CPPUNIT_ASSERT( vi->Command("d$"));
  
  // Test back command.
  stc->SetText("xxxxxxxxxx second\nxxxxxxxx\naaaaaaaaaa\n");
  CPPUNIT_ASSERT( vi->Command(":1"));
  CPPUNIT_ASSERT( vi->Command("yw"));
  CPPUNIT_ASSERT( vi->GetSelectedText() == "xxxxxxxxxx");
  
  CPPUNIT_ASSERT( vi->Command("w"));
  CPPUNIT_ASSERT( vi->Command("x"));
  CPPUNIT_ASSERT(!stc->GetText().Contains("second"));
  CPPUNIT_ASSERT( vi->Command("p"));
  // this is different from vi, that would put back s after e: escond
  CPPUNIT_ASSERT( stc->GetText().Contains("second"));
  
  // Test other commands.
  CPPUNIT_ASSERT( vi->Command("dG"));
  CPPUNIT_ASSERT( vi->Command("dgg"));
  stc->SetText("xxxxxxxxxx second\nxxxxxxxx\naaaaaaaaaa\n");
  CPPUNIT_ASSERT( vi->Command("fx"));
  CPPUNIT_ASSERT( vi->Command("Fx"));
  CPPUNIT_ASSERT( vi->Command(";"));
  
  CPPUNIT_ASSERT( vi->Command("gg"));
  
  CPPUNIT_ASSERT( vi->Command("yw"));
  
  CPPUNIT_ASSERT( vi->Command("zc"));
  CPPUNIT_ASSERT( vi->Command("zo"));
  CPPUNIT_ASSERT( vi->Command("zE"));
  CPPUNIT_ASSERT( vi->Command(">>"));
  CPPUNIT_ASSERT( vi->Command("<<"));

  // Special put test. 
  // Put should not put text within a line, but after it, or before it.
  stc->SetText("the chances of anything coming from mars\n");
  vi->Command("$");
  vi->Command("h");
  vi->Command("h");
  vi->Command("yy");
  CPPUNIT_ASSERT( 
    wxString(stc->GetVi().GetMacros().GetRegister('0')).Contains("the chances of anything"));
  vi->Command("p");
  vi->Command("P");
  CPPUNIT_ASSERT( stc->GetText().Contains(
    "the chances of anything coming from mars"));
  CPPUNIT_ASSERT(!stc->GetText().Contains("mathe"));
 
  // Test macro.
  // First load macros.
  CPPUNIT_ASSERT( wxExViMacros::LoadDocument());
  
  stc->SetText("this text contains xx");
  CPPUNIT_ASSERT( vi->Command("qt"));
  CPPUNIT_ASSERT( vi->Command("/xx"));
  CPPUNIT_ASSERT( vi->Command("rz"));
  CPPUNIT_ASSERT( vi->Command("q"));
  
  CPPUNIT_ASSERT( vi->Command("@t"));
  CPPUNIT_ASSERT( vi->Command("@@"));
  CPPUNIT_ASSERT( vi->Command("."));
  CPPUNIT_ASSERT( vi->Command("10@t"));
  
  // Next should be OK, but crashes due to input expand variable.
  //CPPUNIT_ASSERT( vi->Command("@hdr@"));
  
  // Test variable.
  stc->SetText("");
  CPPUNIT_ASSERT( vi->Command("@Date@"));
  CPPUNIT_ASSERT(!stc->GetText().Contains("Date"));
  stc->SetText("");
  CPPUNIT_ASSERT( vi->Command("@Year@"));
  CPPUNIT_ASSERT( stc->GetText().Contains("20"));
//  CPPUNIT_ASSERT(!vi->Command("@xxx@"));
  CPPUNIT_ASSERT( stc->SetLexer("cpp"));
  stc->SetText("");
  CPPUNIT_ASSERT( vi->Command("@Cb@"));
  CPPUNIT_ASSERT( vi->Command("@Ce@"));
  CPPUNIT_ASSERT( stc->GetText().Contains("//"));
  stc->SetText("");
  CPPUNIT_ASSERT( vi->Command("@Cl@"));
  CPPUNIT_ASSERT( stc->GetText().Contains("//"));
  CPPUNIT_ASSERT( vi->Command("@Nl@"));
  
  // Test illegal command.
  CPPUNIT_ASSERT(!vi->Command("dx"));
  CPPUNIT_ASSERT( vi->GetLastCommand() != "dx");
  CPPUNIT_ASSERT( vi->Command(ESC));
  
  // Test registers
  stc = new wxExSTC(wxTheApp->GetTopWindow(), wxExFileName(TEST_FILE));
  vi = &stc->GetVi();
  const int ctrl_r = 18;
  CPPUNIT_ASSERT( vi->Command("i"));
  CPPUNIT_ASSERT( vi->Command(wxString(wxUniChar(ctrl_r)).ToStdString()));
  CPPUNIT_ASSERT( vi->Command("_"));
  CPPUNIT_ASSERT( vi->Command(ESC));
  
  CPPUNIT_ASSERT( vi->Command("i"));
  CPPUNIT_ASSERT( vi->Command(wxString(wxUniChar(ctrl_r)).ToStdString()));
  CPPUNIT_ASSERT( vi->Command("%"));
  CPPUNIT_ASSERT( vi->Command(ESC));
  CPPUNIT_ASSERT( stc->GetText().Contains("test.h"));
  
  CPPUNIT_ASSERT( vi->Command("yy"));
  stc->SetText("");
  CPPUNIT_ASSERT( vi->Command("i"));
  CPPUNIT_ASSERT( vi->Command(wxString(wxUniChar(ctrl_r)).ToStdString()));
  CPPUNIT_ASSERT( vi->Command("0"));
  CPPUNIT_ASSERT( vi->Command(ESC));
  CPPUNIT_ASSERT( stc->GetText().Contains("test.h"));
  
  stc->SetText("XXXXX");
  CPPUNIT_ASSERT( vi->Command("dd"));
  CPPUNIT_ASSERT( vi->Command("i"));
  CPPUNIT_ASSERT( vi->Command(wxString(wxUniChar(ctrl_r)).ToStdString()));
  CPPUNIT_ASSERT( vi->Command("1"));
  CPPUNIT_ASSERT( vi->Command(ESC));
  CPPUNIT_ASSERT( stc->GetText().Contains("XXXXX"));
  
  stc->SetText("YYYYY");
  CPPUNIT_ASSERT( vi->Command("dd"));
  CPPUNIT_ASSERT( vi->Command("i"));
  CPPUNIT_ASSERT( vi->Command(wxString(wxUniChar(ctrl_r)).ToStdString()));
  CPPUNIT_ASSERT( vi->Command("2"));
  CPPUNIT_ASSERT( vi->Command(ESC));
  CPPUNIT_ASSERT( stc->GetText().Contains("XXXXX"));
  
  // Test MODE_VISUAL and MODE_VISUAL_LINE.
  CPPUNIT_ASSERT( vi->GetMode() == wxExVi::MODE_NORMAL);
  event.m_uniChar = 'v';
  CPPUNIT_ASSERT(!vi->OnChar(event));
  CPPUNIT_ASSERT( vi->GetMode() == wxExVi::MODE_VISUAL);
  CPPUNIT_ASSERT( vi->Command(ESC));
  CPPUNIT_ASSERT( vi->GetMode() == wxExVi::MODE_NORMAL);
  event.m_uniChar = 'V';
  CPPUNIT_ASSERT(!vi->OnChar(event));
  CPPUNIT_ASSERT( vi->GetMode() == wxExVi::MODE_VISUAL_LINE);
  CPPUNIT_ASSERT( vi->Command(ESC));
  CPPUNIT_ASSERT( vi->GetMode() == wxExVi::MODE_NORMAL);
  
  CPPUNIT_ASSERT( vi->Command("v"));
  CPPUNIT_ASSERT( vi->GetMode() == wxExVi::MODE_VISUAL);
  CPPUNIT_ASSERT( vi->Command("jjj"));
  CPPUNIT_ASSERT( vi->GetMode() == wxExVi::MODE_VISUAL);
  CPPUNIT_ASSERT( vi->Command(ESC));
  CPPUNIT_ASSERT( vi->GetMode() == wxExVi::MODE_NORMAL);
  CPPUNIT_ASSERT( vi->Command("V"));
  CPPUNIT_ASSERT( vi->GetMode() == wxExVi::MODE_VISUAL_LINE);
  CPPUNIT_ASSERT( vi->Command("jjj"));
  CPPUNIT_ASSERT( vi->GetMode() == wxExVi::MODE_VISUAL_LINE);
  CPPUNIT_ASSERT( vi->Command(ESC));
  CPPUNIT_ASSERT( vi->GetMode() == wxExVi::MODE_NORMAL);
  
  // Test goto.
  stc->SetText("a\nb\nc\nd\ne\nf\ng\nh\ni\nj\nk\n");
  CPPUNIT_ASSERT( stc->GetLineCount() == 12);
  stc->GotoLine(2);
  CPPUNIT_ASSERT( vi->Command("gg"));
  CPPUNIT_ASSERT( stc->GetCurrentLine() == 0);
  CPPUNIT_ASSERT( vi->Command("1G"));
  CPPUNIT_ASSERT( stc->GetCurrentLine() == 0);
  CPPUNIT_ASSERT( vi->Command("10G"));
  CPPUNIT_ASSERT( stc->GetCurrentLine() == 9);
  CPPUNIT_ASSERT( vi->Command("10000G"));
  CPPUNIT_ASSERT( stc->GetCurrentLine() == 11);
}
  
void wxExGuiTestFixture::testViMacros()
{
  wxExSTC* stc = new wxExSTC(wxTheApp->GetTopWindow(), "hello");
  wxExVi* vi = &stc->GetVi();
  const int esc = 27;
  
  wxExViMacros macros;
  
  // Load, save document is last test, to be able to check contents.
  CPPUNIT_ASSERT(!macros.GetFileName().GetFullPath().empty());
  CPPUNIT_ASSERT( wxExViMacros::LoadDocument());
  
  CPPUNIT_ASSERT(!macros.IsModified());
  CPPUNIT_ASSERT(!macros.IsRecording());
  
  macros.StartRecording("a");
  CPPUNIT_ASSERT( macros.IsModified());
  CPPUNIT_ASSERT( macros.IsRecording());
  CPPUNIT_ASSERT(!macros.IsRecorded("a"));
  
  macros.StopRecording();
  CPPUNIT_ASSERT(!macros.IsRecording());
  CPPUNIT_ASSERT( macros.IsModified());
  CPPUNIT_ASSERT(!macros.IsRecorded("a")); // still no macro
  CPPUNIT_ASSERT( macros.GetMacro().empty());
  
  macros.StartRecording("a");
  macros.Record("a");
  macros.Record("test");
  macros.Record(ESC);
  macros.StopRecording();
  
  CPPUNIT_ASSERT( macros.IsModified());
  CPPUNIT_ASSERT(!macros.IsRecording());
  CPPUNIT_ASSERT( macros.IsRecorded("a"));
  CPPUNIT_ASSERT( macros.GetMacro() == "a");
  
  CPPUNIT_ASSERT(!macros.IsRecorded("b"));
  
  stc->SetText("");
  CPPUNIT_ASSERT( macros.Playback(vi, "a"));
  CPPUNIT_ASSERT( macros.Get("a").front() == "a");
  CPPUNIT_ASSERT( stc->GetText().Contains("test"));
  stc->SetText("");
  CPPUNIT_ASSERT(!macros.Playback(vi, "a", 0));
  CPPUNIT_ASSERT(!macros.Playback(vi, "a", -8));
  CPPUNIT_ASSERT(!stc->GetText().Contains("test"));
  CPPUNIT_ASSERT( macros.Playback(vi, "a", 10));
  CPPUNIT_ASSERT( stc->GetText().Contains("testtesttesttest"));
  
  CPPUNIT_ASSERT(!macros.Playback(vi, "b"));
  
  CPPUNIT_ASSERT(!macros.Get().empty());
  
  // Test append to macro.
  CPPUNIT_ASSERT( vi->Command(ESC));
  macros.StartRecording("A");
  macros.Record("w");
  macros.Record("/test");
  macros.StopRecording();
  
  CPPUNIT_ASSERT(!macros.IsRecorded("A"));
  CPPUNIT_ASSERT( macros.Get("a").front() == "a");
  
  // Test recursive macro.
  CPPUNIT_ASSERT( vi->Command(ESC));
  macros.StartRecording("A");
  macros.Record("@");
  macros.Record("a");
  macros.StopRecording();
  
  CPPUNIT_ASSERT(!macros.Playback(vi, "a"));
  
  // Test variables.
  //CPPUNIT_ASSERT(!macros.Expand(vi, "xxx"));

  // Test all builtin macro variables.
  CPPUNIT_ASSERT( macros.Expand(vi, "Cb"));
  CPPUNIT_ASSERT( macros.Expand(vi, "Cc"));
  CPPUNIT_ASSERT( macros.Expand(vi, "Ce"));
  CPPUNIT_ASSERT( macros.Expand(vi, "Cl"));
  CPPUNIT_ASSERT( macros.Expand(vi, "Created"));
  CPPUNIT_ASSERT( macros.Expand(vi, "Date"));
  CPPUNIT_ASSERT( macros.Expand(vi, "Datetime"));
  CPPUNIT_ASSERT( macros.Expand(vi, "Filename"));
  CPPUNIT_ASSERT( macros.Expand(vi, "Fullpath"));
  CPPUNIT_ASSERT( macros.Expand(vi, "Nl"));
  CPPUNIT_ASSERT( macros.Expand(vi, "Path"));
  CPPUNIT_ASSERT( macros.Expand(vi, "Time"));
  CPPUNIT_ASSERT( macros.Expand(vi, "Year"));
  
  // Test environment macro variables.
  CPPUNIT_ASSERT( macros.Expand(vi, "Home"));

  // Test input macro variables.
  // Next requires input...    
  //  CPPUNIT_ASSERT( macros.Expand(vi, "author"));

  // Test template macro variables.
  //wxString value;
  //CPPUNIT_ASSERT( macros.Expand(vi, "cht", value));
  //CPPUNIT_ASSERT( value.Contains("Template example"));

  // So save as last test.
  CPPUNIT_ASSERT( wxExViMacros::SaveDocument());
  
  CPPUNIT_ASSERT(!macros.IsModified());
  
  // A second save is not necessary.
  CPPUNIT_ASSERT(!macros.SaveDocument());
  
  // Test registers.
  CPPUNIT_ASSERT(!macros.GetRegister('a').empty());
  CPPUNIT_ASSERT( macros.GetRegister('z').empty());
  CPPUNIT_ASSERT(!macros.GetRegisters().empty());
  CPPUNIT_ASSERT( macros.Get("z").empty());
  macros.SetRegister('z', "hello z");
  CPPUNIT_ASSERT(!macros.GetRegister('z').empty());
  CPPUNIT_ASSERT(!macros.Get("z").empty());
  CPPUNIT_ASSERT( macros.GetRegister('z') == "hello z");
  macros.SetRegister('Z', " and more");
  CPPUNIT_ASSERT( macros.GetRegister('Z').empty());
  CPPUNIT_ASSERT( macros.GetRegister('z') == "hello z and more");
}
  
wxExAppTestSuite::wxExAppTestSuite()
  : CppUnit::TestSuite("wxExtension test suite")
{
  addTest(new CppUnit::TestCaller<wxExGuiTestFixture>(
    "testAddress",
    &wxExGuiTestFixture::testAddress));
    
  addTest(new CppUnit::TestCaller<wxExGuiTestFixture>(
    "testAddressRange",
    &wxExGuiTestFixture::testAddressRange));
    
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
    "testEx",
    &wxExGuiTestFixture::testEx));

  addTest(new CppUnit::TestCaller<wxExGuiTestFixture>(
    "testFileDialog",
    &wxExGuiTestFixture::testFileDialog));

  addTest(new CppUnit::TestCaller<wxExGuiTestFixture>(
    "testFileStatistics",
    &wxExGuiTestFixture::testFileStatistics));

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
    "testSTCEntryDialog",
    &wxExGuiTestFixture::testSTCEntryDialog));
    
  addTest(new CppUnit::TestCaller<wxExGuiTestFixture>(
    "testSTCFile",
    &wxExGuiTestFixture::testSTCFile));
    
  addTest(new CppUnit::TestCaller<wxExGuiTestFixture>(
    "testStyle",
    &wxExGuiTestFixture::testStyle));
    
  addTest(new CppUnit::TestCaller<wxExGuiTestFixture>(
    "testTextFile",
    &wxExGuiTestFixture::testTextFile));

  addTest(new CppUnit::TestCaller<wxExGuiTestFixture>(
    "testToVectorString",
    &wxExGuiTestFixture::testToVectorString));
    
  addTest(new CppUnit::TestCaller<wxExGuiTestFixture>(
    "testUtil",
    &wxExGuiTestFixture::testUtil));
    
  addTest(new CppUnit::TestCaller<wxExGuiTestFixture>(
    "testVariable",
    &wxExGuiTestFixture::testVariable));
    
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
    "testVersion",
    &wxExGuiTestFixture::testVersion));
    
  addTest(new CppUnit::TestCaller<wxExGuiTestFixture>(
    "testVi",
    &wxExGuiTestFixture::testVi));
    
  addTest(new CppUnit::TestCaller<wxExGuiTestFixture>(
    "testViMacros",
    &wxExGuiTestFixture::testViMacros));
}

wxExAppTestSuite::~wxExAppTestSuite()
{
  // Remove files.
  CPPUNIT_ASSERT(remove("test-ex.txt") == 0);
  CPPUNIT_ASSERT(remove("test.hex") == 0);
}
