////////////////////////////////////////////////////////////////////////////////
// Name:      test-ex.cpp
// Purpose:   Implementation for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/ex.h>
#include <wx/extension/stc.h>
#include <wx/extension/vimacros.h>
#include "test.h"

void wxExGuiTestFixture::testEx()
{
  // Test modeline.
  wxExSTC* stc = new wxExSTC(wxTheApp->GetTopWindow(), 
    "// vi: set ts=120 ec=40");
    
  CPPUNIT_ASSERT(stc->GetTabWidth() == 120);
  CPPUNIT_ASSERT(stc->GetEdgeColumn() == 40);
  
  wxExEx* ex = new wxExEx(stc);
  
  CPPUNIT_ASSERT( ex->GetIsActive());
  ex->Use(false);
  CPPUNIT_ASSERT(!ex->GetIsActive());
  ex->Use(true);
  CPPUNIT_ASSERT( ex->GetIsActive());
  
  CPPUNIT_ASSERT( ex->GetMacros().GetCount() > 0);

  // Test last command.  
  CPPUNIT_ASSERT( ex->Command(":.="));
  CPPUNIT_ASSERT( ex->GetLastCommand() == ":set ts=120 ec=40");
  CPPUNIT_ASSERT(!ex->Command(":xxx"));
  CPPUNIT_ASSERT( ex->GetLastCommand() == ":set ts=120 ec=40");
  CPPUNIT_ASSERT(!ex->Command(":yyy"));
  CPPUNIT_ASSERT( ex->GetLastCommand() == ":set ts=120 ec=40");
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
  CPPUNIT_ASSERT(!ex->Command(":'<,'>x"));
  
  // Test set.
  CPPUNIT_ASSERT( ex->Command(":set"));
  CPPUNIT_ASSERT( ex->Command(":set ec=5"));
  CPPUNIT_ASSERT( ex->Command(":set sy=cpp"));
  CPPUNIT_ASSERT( ex->Command(":set ts=10"));
  CPPUNIT_ASSERT(!ex->Command(":set xxx"));
  
  const std::vector<std::string> switches{
    "ai", "ac", "el", "ic", "ln", "mw", "re", "wl", "ws"};
  
  for (const auto& it : switches)
  {
    CPPUNIT_ASSERT( ex->Command(":set " + it));
    CPPUNIT_ASSERT( ex->Command(":set " + it + "!"));
  }
  
  CPPUNIT_ASSERT( ex->Command(":set ai")); // back to default
  
  CPPUNIT_ASSERT( ex->Command(":d"));
  //CPPUNIT_ASSERT( ex->Command(":e")); // shows dialog
  CPPUNIT_ASSERT(!ex->Command(":n"));
  CPPUNIT_ASSERT(!ex->Command(":prev"));
  CPPUNIT_ASSERT( ex->Command(":grep test"));
  CPPUNIT_ASSERT( ex->Command(":sed"));
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

  // Test registers.  
  ex->SetRegistersDelete("x");
  ex->SetRegisterYank("test");
  CPPUNIT_ASSERT( ex->GetMacros().GetRegister('0') == "test");
  CPPUNIT_ASSERT( ex->GetRegisterText() == "test");
  ex->SetRegisterInsert("insert");
  CPPUNIT_ASSERT( ex->GetRegisterInsert() == "insert");
  
  stc->SetText("the chances");
  stc->SelectAll();
  ex->Yank();
  CPPUNIT_ASSERT( ex->GetRegisterText() == "the chances");
  ex->Cut();
  CPPUNIT_ASSERT( ex->GetRegisterText() == "the chances");
  CPPUNIT_ASSERT( ex->GetMacros().GetRegister('1') == "the chances");
  CPPUNIT_ASSERT( ex->GetSelectedText().empty());
}
