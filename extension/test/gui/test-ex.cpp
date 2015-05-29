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

//#define DEBUGGING ON

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
  
  CPPUNIT_ASSERT( ex->GetIsActive());
  ex->Use(false);
  CPPUNIT_ASSERT(!ex->GetIsActive());
  ex->Use(true);
  CPPUNIT_ASSERT( ex->GetIsActive());
  
  stc->SetText("xx\nyy\nzz\n");
  stc->DocumentStart();

  // Test valid commands and last command.  
  for (const auto& command : std::vector<std::pair<std::string, bool>> {
    {":ab",true},
    {":ve",false},
    {":reg",true},
    {":set",true},
    {":10",true},
    {":.=",true},
    {":/yy/=",true},
    {":.kz",true},
    {":.pu",true},
    {":.puz",true},
    {":g/is/s//ok",true},
    {":g/is/d",true},
    {":g/is/p",true},
    {":%g/is/p",true},
    {":1,2g/is/p",true},
    {":2",true},
    {":.m$",true},
    {":2",true},
    {":.t$",true},
    {":%s/x/y",true},
    {":%s/z/z",true},
    {":.s/$/\n",true},
    {":.S",true},
    {":.S10",true},
    {":.Sr",true},
    {":.Su",true},
    {":.Sru",true},
    {":.S10r",true},
    {":.S10u",true},
    {":.S1,5u",true},
    {":1,$s/this/ok",true},
    {":1,$s/$/ZXXX/",true},
    {":1,$s/$/ZXXX/",true},
    {":1,$s/^/Zxxx/",true},
    {":1,$&",true},
    {":1,$&g",true},
    {":1,$~",true},
    {":1,$~g",true},
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
    
  // Test invalid commands.  
  for (const auto& command : std::vector<std::string> {
    // We have only one document, so :n, :prev return false.
    ":n",
    ":prev",
    ":.k",
    "set xxx",
    "so",
    "so xxx",
    ":xxx",
    ":zzz",
    ":%/test//",
    ":1,$k",
    ":.S0",
    ":.Sx"})
  {
    CPPUNIT_ASSERT_MESSAGE(command, !ex->Command(command));
    CPPUNIT_ASSERT( ex->GetLastCommand() != command);
  }
  
  stc->SetText("xx\nyy\nzz\n");
  CPPUNIT_ASSERT( ex->Command(":/xx/,/yy/y"));
  
  CPPUNIT_ASSERT( ex->Command(":1"));
  CPPUNIT_ASSERT( ex->MarkerAdd('t'));
  CPPUNIT_ASSERT( ex->Command(":$"));
  CPPUNIT_ASSERT( ex->MarkerAdd('u'));
  CPPUNIT_ASSERT( ex->Command(":'t,'us/s/w/"));
  CPPUNIT_ASSERT( ex->GetLastCommand() == ":'t,'us/s/w/");
  
  // Test read (file does not exist).
  CPPUNIT_ASSERT(!ex->Command(":r test-ex.txt"));
  
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
  
  // Test execute.
  CPPUNIT_ASSERT( ex->Command(":!pwd"));
  CPPUNIT_ASSERT( ex->Command(":!bash"));
  
  // Test set options.
  for (const auto& option : std::vector<std::pair<std::string, std::string>> {
    {"ec","5"},{"sy","cpp"},{"ts","10"}})
  {
    CPPUNIT_ASSERT( ex->Command(":set " + option.first + "=" + option.second));
  }
  
  // Test set switches.
  for (const auto& it :  std::vector<std::string> {
    "ai", "ac", "el", "ic", "mw", "nu", "re", "wl", "ws"})
  {
    CPPUNIT_ASSERT_MESSAGE( it, ex->Command(":set " + it));
    CPPUNIT_ASSERT_MESSAGE( it, ex->Command(":set " + it + "!"));
  }
  
  CPPUNIT_ASSERT( ex->Command(":set ai")); // back to default
  
  // Test source.
  CPPUNIT_ASSERT( ex->Command(":so test-source.txt"));
  
  CPPUNIT_ASSERT( ex->Command(":d"));
  //CPPUNIT_ASSERT( ex->Command(":e")); // shows dialog
  CPPUNIT_ASSERT(!ex->Command(":n"));
  CPPUNIT_ASSERT(!ex->Command(":prev"));
  CPPUNIT_ASSERT( ex->Command(":grep test"));
  CPPUNIT_ASSERT( ex->Command(":sed"));
  CPPUNIT_ASSERT( ex->Command(":r !echo qwerty"));
  CPPUNIT_ASSERT( stc->GetText().Contains("qwerty"));
  CPPUNIT_ASSERT( ex->Command(":y"));
  CPPUNIT_ASSERT( ex->Command(":1,$s/^/BEGIN-OF-LINE"));
  CPPUNIT_ASSERT( ex->Command(":w test-ex.txt"));
  CPPUNIT_ASSERT( ex->Command(":1,2w test-ex.txt"));
  CPPUNIT_ASSERT( ex->Command(":1,2w >> test-ex.txt"));
  CPPUNIT_ASSERT( ex->Command(":1,2w >> test-ex.txt"));
  CPPUNIT_ASSERT( ex->Command(":r test-ex.txt"));
  CPPUNIT_ASSERT( ex->Command(":$r test-ex.txt"));

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
  
  // Test print.
  ex->Print("This is printed");
  
  // Test delete.
  stc->SetText("a\nb\nc\nd\ne\nf\ng\nh\ni\nj\nk\n");
  CPPUNIT_ASSERT( ex->Command(":10d"));

  // Test global delete (previous delete was on found text).
  const int max = 10;
  for (int i = 0; i < max; i++) stc->AppendText("line xxxx added\n");
  const int lines = stc->GetLineCount();
  CPPUNIT_ASSERT( ex->Command(":g/xxxx/d"));
  CPPUNIT_ASSERT_MESSAGE(std::to_string(stc->GetLineCount()) + "!=" + 
    std::to_string(lines - max), stc->GetLineCount() == lines - max);
  
  // Test global print.
  stc->AppendText("line xxxx 3 added\n");
  stc->AppendText("line xxxx 4 added\n");
  CPPUNIT_ASSERT( ex->Command(":g/xxxx/p"));
  CPPUNIT_ASSERT( ex->Command(":g/xxxx/p#"));
  CPPUNIT_ASSERT( ex->Command(":g//"));
  
  // Test global substitute.
  stc->AppendText("line xxxx 6 added\n");
  stc->AppendText("line xxxx 7 added\n");
  CPPUNIT_ASSERT( ex->Command(":g/xxxx/s//yyyy"));
  CPPUNIT_ASSERT( stc->GetText().Contains("yyyy"));
  CPPUNIT_ASSERT( ex->Command(":g//"));
  
  // Test global move.
  stc->SetText("a\nb\nc\nd\ne\nf\ng\nh\ni\nj\nk\n");
  CPPUNIT_ASSERT( ex->Command(":g/d/m$"));
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
