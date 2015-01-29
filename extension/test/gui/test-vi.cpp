////////////////////////////////////////////////////////////////////////////////
// Name:      test-vi.cpp
// Purpose:   Implementation for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/vi.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/vimacros.h>
#include <wx/extension/stc.h>
#include "test.h"

#define ESC "\x1b"

void wxExGuiTestFixture::testVi()
{
  const int esc = 27;
 
  // Test for modeline support.
  wxExSTC* stc = new wxExSTC(m_Frame, 
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
  CPPUNIT_ASSERT( vi->GetRegisterInsert() == "xxxxxxxx");
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
  CPPUNIT_ASSERT(!stc->GetModify());
  
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
  stc = new wxExSTC(m_Frame, GetTestFile());
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
  
  // Test visual modes.
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
  event.m_uniChar = 'Z';
  CPPUNIT_ASSERT(!vi->OnChar(event));
  CPPUNIT_ASSERT( vi->GetMode() == wxExVi::MODE_VISUAL_RECT);
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
  
  CPPUNIT_ASSERT( vi->Command("Z"));
  CPPUNIT_ASSERT( vi->GetMode() == wxExVi::MODE_VISUAL_RECT);
  CPPUNIT_ASSERT( vi->Command("jjj"));
  CPPUNIT_ASSERT( vi->GetMode() == wxExVi::MODE_VISUAL_RECT);
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
