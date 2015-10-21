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
#include <wx/extension/managedframe.h>
#include <wx/extension/stc.h>
#include <wx/extension/vimacros.h>
#include "test.h"

void fixture::testEx()
{
  // Test modeline.
  const wxString modeline("set ts=120 ec=40 sy=sql");
  wxExSTC* stc = new wxExSTC(m_Frame, "-- vi: " + modeline);
  m_Frame->GetManager().AddPane(stc, wxAuiPaneInfo().CenterPane());
  wxExEx* ex = new wxExEx(stc);
    
  CPPUNIT_ASSERT_MESSAGE(std::to_string(stc->GetTabWidth()), stc->GetTabWidth() == 120);
  CPPUNIT_ASSERT(stc->GetEdgeColumn() == 40);
  CPPUNIT_ASSERT(stc->GetLexer().GetScintillaLexer() == "sql");
  CPPUNIT_ASSERT( ex->GetLastCommand() == ":" + modeline);
  wxExSTC* stc2 = new wxExSTC(m_Frame, wxExFileName("test-modeline.txt"));
  m_Frame->GetManager().AddPane(stc2, wxAuiPaneInfo().Bottom().Caption("STC"));
  CPPUNIT_ASSERT(stc2->GetLexer().GetScintillaLexer() == "sql");
  m_Frame->GetManager().Update();
  
  stc->SetText("xx\nxx\nyy\nzz\n");
  stc->DocumentStart();
  
  // AddText
  ex->AddText(" added");
  CPPUNIT_ASSERT( stc->GetText().Contains("added"));
  
  // GetFrame
  CPPUNIT_ASSERT( ex->GetFrame() == m_Frame);

  // GetIsActive
  CPPUNIT_ASSERT( ex->GetIsActive());
  ex->Use(false);
  CPPUNIT_ASSERT(!ex->GetIsActive());
  ex->Use(true);
  CPPUNIT_ASSERT( ex->GetIsActive());
  
  // GetSearchFlags
  CPPUNIT_ASSERT( ex->GetSearchFlags() & wxSTC_FIND_REGEXP);
  
  // Test valid Commands and GetLastCommand. 
  // Most valid commands are tested using the :so command.
  for (const auto& command : std::vector<std::pair<std::string, bool>> {
    {":ab",true},
    {":ve",false},
    {":1,$s/s/w/",true}})
  {
    CPPUNIT_ASSERT_MESSAGE( command.first, ex->Command(command.first));
      
    if (command.second)
    {
      if (command.first != ":.=")
      {
        CPPUNIT_ASSERT_MESSAGE( command.first, ex->GetLastCommand() == command.first);
      }
    }
    else
    {
      CPPUNIT_ASSERT( ex->GetLastCommand() != command.first);
    }
  }
    
  ex->AddText("XXX");
  
  // Test invalid commands.  
  for (const auto& command : std::vector<std::string> {
    // We have only one document, so :n, :prev return false.
    ":n",
    ":prev",
    ":.k",
    ":pk",
    ":.pk",
    "set xxx",
    "so",
    "so xxx",
    ":xxx",
    ":zzz",
    ":%/test//",
    ":1,$k",
    ":.S0",
    ":.Sx",
    ":/XXX/x",
    ":r test-ex.txt"})
  {
    CPPUNIT_ASSERT_MESSAGE(command, !ex->Command(command));
    CPPUNIT_ASSERT( ex->GetLastCommand() != command);
  }
  
  // Test abbreviations.
  stc->SetText("xx\n");
  CPPUNIT_ASSERT( ex->Command(":ab t TTTT"));
  const auto& it1 = ex->GetMacros().GetAbbreviations().find("t");
  CPPUNIT_ASSERT (it1 != ex->GetMacros().GetAbbreviations().end());
  CPPUNIT_ASSERT( it1->second == "TTTT");
  CPPUNIT_ASSERT( ex->Command(":una t"));
  const auto& it2 = ex->GetMacros().GetAbbreviations().find("t");
  CPPUNIT_ASSERT (it2 == ex->GetMacros().GetAbbreviations().end());
  
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
  
  // Test source.
  stc->SetText("xx\nxx\nyy\nzz\n");
  CPPUNIT_ASSERT( ex->Command(":so test-source.txt"));
  stc->SetText("xx\nxx\nyy\nzz\n");
  CPPUNIT_ASSERT( ex->Command(":source test-source.txt"));
  stc->SetText("xx\nxx\nyy\nzz\n");
  CPPUNIT_ASSERT(!ex->Command(":so test-surce.txt"));
  stc->SetText("xx\nxx\nyy\nzz\n");
  CPPUNIT_ASSERT(!ex->Command(":so test-source-2.txt"));
  
  CPPUNIT_ASSERT( ex->Command(":d"));
  CPPUNIT_ASSERT( ex->Command(":r !echo qwerty"));
  CPPUNIT_ASSERT( stc->GetText().Contains("qwerty"));

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
  
  ex->MacroRecord("a");
  CPPUNIT_ASSERT(!ex->GetMacros().IsRecording()); // if not recording it does not start it
  CPPUNIT_ASSERT(!ex->GetMacros().IsRecorded("a"));
  
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
  stc->SetText("xx\nyy\nzz\n");
  CPPUNIT_ASSERT( ex->Command(":1"));
  CPPUNIT_ASSERT( ex->MarkerAdd('t'));
  CPPUNIT_ASSERT( ex->Command(":$"));
  CPPUNIT_ASSERT( ex->MarkerAdd('u'));
  CPPUNIT_ASSERT( ex->Command(":'t,'us/s/w/"));
  CPPUNIT_ASSERT( ex->GetLastCommand() == ":'t,'us/s/w/");
  
  // Test print.
  ex->Print("This is printed");
  
  // Test global delete (previous delete was on found text).
  const int max = 10;
  for (int i = 0; i < max; i++) stc->AppendText("line xxxx added\n");
  const int lines = stc->GetLineCount();
  CPPUNIT_ASSERT( ex->Command(":g/xxxx/d"));
  CPPUNIT_ASSERT_MESSAGE(std::to_string(stc->GetLineCount()) + "!=" + 
    std::to_string(lines - max), stc->GetLineCount() == lines - max);
  
  // Test global substitute.
  stc->AppendText("line xxxx 6 added\n");
  stc->AppendText("line xxxx 7 added\n");
  CPPUNIT_ASSERT( ex->Command(":g/xxxx/s//yyyy"));
  CPPUNIT_ASSERT( stc->GetText().Contains("yyyy"));
  CPPUNIT_ASSERT( ex->Command(":g//"));
  
  // Test global move.
  stc->SetText("a\nb\nc\nd\ne\nf\ng\nh\ni\nj\nk\n");
  CPPUNIT_ASSERT(!ex->Command(":g/d/m$")); // possible infinite loop
  CPPUNIT_ASSERT( stc->GetText().Contains("d"));
  
  // Test substitute.
  stc->SetText("we have xxxx yyyy zzzz");
  CPPUNIT_ASSERT( ex->Command(":set re"));
  CPPUNIT_ASSERT( ex->Command(":%s/ccccc/ddd"));
  CPPUNIT_ASSERT( ex->Command(":%s/\\(x+\\) *\\(y+\\)/\\\\2 \\\\1"));
  CPPUNIT_ASSERT( stc->GetText() == "we have yyyy xxxx zzzz");
  stc->SetText("we have xxxx 'zzzz'");
  CPPUNIT_ASSERT( ex->Command(":%s/'//g"));
  CPPUNIT_ASSERT_MESSAGE(stc->GetText().ToStdString(), stc->GetText() == "we have xxxx zzzz" );
  CPPUNIT_ASSERT(!ex->Command(":.s/x*//g"));
  CPPUNIT_ASSERT(!ex->Command(":.s/ *//g"));
  
  // Test goto.
  stc->SetText("a\nb\nc\nd\ne\nf\ng\nh\ni\nj\nk\n");
  CPPUNIT_ASSERT( stc->GetLineCount() == 12);
  stc->GotoLine(2);

  for (auto& go : std::vector<std::pair<std::string, int>> {
    {":1",0},
    {":-10",0},
    {":10",9},
    {":/c/",2},
    {":10000",11}})
  {
    CPPUNIT_ASSERT(  ex->Command(go.first));
    CPPUNIT_ASSERT( stc->GetCurrentLine() == go.second);
  }
  
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
