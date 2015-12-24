////////////////////////////////////////////////////////////////////////////////
// Name:      test-ex.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/ex.h>
#include <wx/extension/filename.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/stc.h>
#include <wx/extension/vimacros.h>
#include "test.h"

TEST_CASE("wxExEx", "[stc][vi]")
{
  // Test modeline.
  const wxString modeline("set ts=120 ec=40 sy=sql");
  wxExSTC* stc = new wxExSTC(GetFrame(), "-- vi: " + modeline);
  AddPane(GetFrame(), stc);
  wxExEx* ex = new wxExEx(stc);

  REQUIRE(stc->GetVi().GetIsActive());
  REQUIRE(stc->GetTabWidth() == 120);
  REQUIRE(stc->GetEdgeColumn() == 40);
  REQUIRE(stc->GetLexer().GetScintillaLexer() == "sql");
  REQUIRE( ex->GetLastCommand() == wxString(":" + modeline));
  wxExSTC* stco = new wxExSTC(GetFrame(), wxExFileName("test-modeline.txt"));
  AddPane(GetFrame(), stco);
  REQUIRE(stco->GetLexer().GetScintillaLexer() == "sql");
  
  stc->SetText("xx\nxx\nyy\nzz\n");
  stc->DocumentStart();
  
  // AddText
  ex->AddText(" added");
  REQUIRE( stc->GetText().Contains("added"));
  
  // GetFrame
  REQUIRE( ex->GetFrame() == GetFrame());

  // GetIsActive
  REQUIRE( ex->GetIsActive());
  ex->Use(false);
  REQUIRE(!ex->GetIsActive());
  ex->Use(true);
  REQUIRE( ex->GetIsActive());
  
  // GetSearchFlags
  REQUIRE( (ex->GetSearchFlags() & wxSTC_FIND_REGEXP));
  
  // Test valid Commands and GetLastCommand. 
  // Most valid commands are tested using the :so command.
  for (const auto& command : std::vector<std::pair<std::string, bool>> {
    {":ab",true},
    {":ve",false},
    {":1,$s/s/w/",true}})
  {
    INFO( command.first);
    REQUIRE( ex->Command(command.first));
      
    if (command.second)
    {
      if (command.first != ":.=")
      {
        INFO( command.first);
        REQUIRE( ex->GetLastCommand() == command.first);
      }
    }
    else
    {
      REQUIRE( ex->GetLastCommand() != command.first);
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
    ":r test-xx.txt"})
  {
    INFO(command);
    REQUIRE(!ex->Command(command));
    REQUIRE( ex->GetLastCommand() != command);
  }
  
  // Test abbreviations.
  stc->SetText("xx\n");
  REQUIRE( ex->Command(":ab t TTTT"));
  const auto& it1 = ex->GetMacros().GetAbbreviations().find("t");
  REQUIRE (it1 != ex->GetMacros().GetAbbreviations().end());
  REQUIRE( it1->second == "TTTT");
  REQUIRE( ex->Command(":una t"));
  const auto& it2 = ex->GetMacros().GetAbbreviations().find("t");
  REQUIRE (it2 == ex->GetMacros().GetAbbreviations().end());
  
  // Test range.
  stc->SetText("a\nb\nc\nd\ne\nf\ng\nh\ni\nj\nk\n");
  REQUIRE( ex->Command(":1,2>"));
  stc->SelectNone();
  REQUIRE(!ex->Command(":'<,'>>"));
  stc->GotoLine(2);
  stc->LineDownExtend();
  REQUIRE( ex->Command(":'<,'>m1"));
  stc->GotoLine(2);
  stc->LineDownExtend();
  REQUIRE( ex->Command(":'<,'>wtest-ex.txt"));
  REQUIRE( ex->Command(":'<,'><"));
  REQUIRE( ex->Command(":'<,'>>"));
  REQUIRE( ex->Command(":'<,'>!sort"));
  stc->GotoLine(2);
  stc->LineDownExtend();
  REQUIRE(!ex->Command(":'<,'>x"));
  
  // Test source.
#ifdef __UNIX
  stc->SetText("xx\nxx\nyy\nzz\n");
  REQUIRE( ex->Command(":so test-source.txt"));
  stc->SetText("xx\nxx\nyy\nzz\n");
  REQUIRE( ex->Command(":source test-source.txt"));
  stc->SetText("xx\nxx\nyy\nzz\n");
  REQUIRE(!ex->Command(":so test-surce.txt"));
  stc->SetText("xx\nxx\nyy\nzz\n");
  REQUIRE(!ex->Command(":so test-source-2.txt"));
  
  REQUIRE( ex->Command(":d"));
  REQUIRE( ex->Command(":r !echo qwerty"));
  REQUIRE( stc->GetText().Contains("qwerty"));
#endif
  
  // Test macros.
  // Do not load macros yet, to test IsRecorded.
  REQUIRE(!ex->GetMacros().IsRecording());
  REQUIRE(!ex->GetMacros().IsRecorded("a"));
  
  ex->MacroStartRecording("a");
  REQUIRE( ex->GetMacros().IsRecording());
  REQUIRE(!ex->GetMacros().IsRecorded("a"));
  
  ex->GetMacros().StopRecording();
  REQUIRE(!ex->GetMacros().IsRecording());
  REQUIRE(!ex->GetMacros().IsRecorded("a")); // still no macro
  
  ex->MacroRecord("a");
  REQUIRE(!ex->GetMacros().IsRecording()); // if not recording it does not start it
  REQUIRE(!ex->GetMacros().IsRecorded("a"));
  
  ex->MacroStartRecording("a");
  REQUIRE( ex->Command(":10"));
  ex->GetMacros().StopRecording();
  
  REQUIRE(!ex->GetMacros().IsRecording());
  REQUIRE( ex->GetMacros().IsRecorded("a"));
  REQUIRE( ex->GetMacros().StartsWith("a"));
  REQUIRE(!ex->GetMacros().StartsWith("b"));
  
  REQUIRE(!ex->GetMacros().IsRecorded("b"));
  
  REQUIRE( ex->MacroPlayback("a"));
//  REQUIRE(!ex->MacroPlayback("b"));
  REQUIRE( ex->GetMacros().GetMacro() == "a");
  REQUIRE( ex->GetSTC() == stc);

  REQUIRE( wxExViMacros::LoadDocument());
  REQUIRE(!ex->GetMacros().IsRecorded("xxx"));
  REQUIRE( ex->GetMacros().GetCount() > 0);
  
  REQUIRE( ex->GetMacros().StartsWith("Da"));
  
  REQUIRE( ex->GetMacros().Expand(ex, "Date"));
//  REQUIRE(!ex->GetMacros().Expand(ex, "xxx"));
  
  // Test markers.
  REQUIRE( ex->MarkerAdd('a'));
  REQUIRE( ex->MarkerLine('a') != -1);
  REQUIRE( ex->MarkerGoto('a'));
  REQUIRE( ex->MarkerDelete('a'));
  REQUIRE(!ex->MarkerDelete('b'));
  REQUIRE(!ex->MarkerGoto('a'));
  REQUIRE(!ex->MarkerDelete('a'));
  stc->SetText("xx\nyy\nzz\n");
  REQUIRE( ex->Command(":1"));
  REQUIRE( ex->MarkerAdd('t'));
  REQUIRE( ex->Command(":$"));
  REQUIRE( ex->MarkerAdd('u'));
  REQUIRE( ex->Command(":'t,'us/s/w/"));
  REQUIRE( ex->GetLastCommand() == ":'t,'us/s/w/");
  
  // Test print.
  ex->Print("This is printed");
  
  // Test global delete (previous delete was on found text).
  const int max = 10;
  for (int i = 0; i < max; i++) stc->AppendText("line xxxx added\n");
  const int lines = stc->GetLineCount();
  REQUIRE( ex->Command(":g/xxxx/d"));
  INFO(std::to_string(stc->GetLineCount()) + "!=" + 
    std::to_string(lines - max));
  REQUIRE(stc->GetLineCount() == lines - max);
  
  // Test global substitute.
  stc->AppendText("line xxxx 6 added\n");
  stc->AppendText("line xxxx 7 added\n");
  REQUIRE( ex->Command(":g/xxxx/s//yyyy"));
  REQUIRE( stc->GetText().Contains("yyyy"));
  REQUIRE( ex->Command(":g//"));
  
  // Test global move.
  stc->SetText("a\nb\nc\nd\ne\nf\ng\nh\ni\nj\nk\n");
  REQUIRE(!ex->Command(":g/d/m$")); // possible infinite loop
  REQUIRE( stc->GetText().Contains("d"));
  
  // Test substitute.
  stc->SetText("we have xxxx yyyy zzzz");
  REQUIRE( ex->Command(":set re"));
  REQUIRE( ex->Command(":%s/ccccc/ddd"));
  REQUIRE( ex->Command(":%s/\\(x+\\) *\\(y+\\)/\\\\2 \\\\1"));
  REQUIRE( stc->GetText() == "we have yyyy xxxx zzzz");
  stc->SetText("we have xxxx 'zzzz'");
  REQUIRE( ex->Command(":%s/'//g"));
  INFO(stc->GetText().ToStdString() );
  REQUIRE(stc->GetText() == "we have xxxx zzzz" );
  REQUIRE(!ex->Command(":.s/x*//g"));
  REQUIRE(!ex->Command(":.s/ *//g"));
  
  // Test goto.
  stc->SetText("a\nb\nc\nd\ne\nf\ng\nh\ni\nj\nk\n");
  REQUIRE( stc->GetLineCount() == 12);
  stc->GotoLine(2);

  for (auto& go : std::vector<std::pair<std::string, int>> {
    {":1",0},
    {":-10",0},
    {":10",9},
    {":/c/",2},
    {":10000",11}})
  {
    REQUIRE(  ex->Command(go.first));
    REQUIRE( stc->GetCurrentLine() == go.second);
  }
  
  // Test registers.  
  ex->SetRegistersDelete("x");
  ex->SetRegisterYank("test");
  REQUIRE( ex->GetMacros().GetRegister('0') == "test");
  REQUIRE( ex->GetRegisterText() == "test");
  ex->SetRegisterInsert("insert");
  REQUIRE( ex->GetRegisterInsert() == "insert");
  
  stc->SetText("the chances");
  stc->SelectAll();
  ex->Yank();
  REQUIRE( ex->GetRegisterText() == "the chances");
  ex->Cut();
  REQUIRE( ex->GetRegisterText() == "the chances");
  REQUIRE( ex->GetMacros().GetRegister('1') == "the chances");
  REQUIRE( ex->GetSelectedText().empty());
}
