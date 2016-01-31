////////////////////////////////////////////////////////////////////////////////
// Name:      test-vi.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/vi.h>
#include <wx/extension/frd.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/vimacros.h>
#include <wx/extension/stc.h>
#include "test.h"

#define ESC "\x1b"

void ChangeMode(wxExVi* vi, const std::string& command, int mode)
{
  INFO( command);
  REQUIRE( vi->Command(command));
  REQUIRE( vi->GetMode() == mode);
}

TEST_CASE("wxExVi", "[stc][vi]")
{
  // Test for modeline support.
  wxExSTC* stc = new wxExSTC(GetFrame(), 
    "// vi: set ts=120 "
    "// this is a modeline");
  AddPane(GetFrame(), stc);
    
  wxExVi* vi = &stc->GetVi();
  
  REQUIRE(stc->GetTabWidth() == 120);
  REQUIRE( vi->GetIsActive());
  REQUIRE( vi->ModeNormal());

  wxKeyEvent event(wxEVT_CHAR);
  
  // Test WXK_NONE.
  event.m_uniChar = WXK_NONE;
  REQUIRE( vi->OnChar(event));
  
  // First i enters insert mode, so is handled by vi, not to be skipped.
  event.m_uniChar = 'i';
  REQUIRE(!vi->OnChar(event));
  REQUIRE( vi->GetMode() == wxExVi::MODE_INSERT);
  
  // Second i (and more) all handled by vi.
  for (int i = 0; i < 10; i++) REQUIRE(!vi->OnChar(event));

  // Repeat some macro tests.
  REQUIRE(!vi->GetMacros().IsRecording());
  vi->MacroStartRecording("a");
  REQUIRE( vi->GetMacros().IsRecording());
  REQUIRE(!vi->GetMacros().IsRecorded("a"));
  vi->GetMacros().StopRecording();
  REQUIRE(!vi->GetMacros().IsRecording());
  REQUIRE(!vi->GetMacros().IsRecorded("a")); // still no macro
  ChangeMode( vi, ESC, wxExVi::MODE_NORMAL);
  vi->MacroStartRecording("a");
  REQUIRE( vi->GetMacros().IsRecording());
  REQUIRE(!vi->OnChar(event));
  REQUIRE( vi->GetMode() == wxExVi::MODE_INSERT);
  REQUIRE(!vi->OnChar(event));
  REQUIRE( vi->GetMode() == wxExVi::MODE_INSERT);
  ChangeMode( vi, ESC, wxExVi::MODE_NORMAL);
  vi->GetMacros().StopRecording();
  REQUIRE(!vi->GetMacros().IsRecording());
  REQUIRE( vi->GetMacros().IsRecorded("a"));
  REQUIRE(!vi->GetMacros().IsRecorded("b"));
  REQUIRE( vi->MacroPlayback("a"));
  ChangeMode( vi, ESC, wxExVi::MODE_NORMAL);

  // Test control keys.
  for (const auto& control_key : std::vector<int> {
    WXK_CONTROL_B,WXK_CONTROL_E,WXK_CONTROL_F,WXK_CONTROL_G,
    WXK_CONTROL_J,WXK_CONTROL_P,WXK_CONTROL_Q})
  {
    event.m_uniChar = control_key;
    REQUIRE( vi->OnKeyDown(event));
    REQUIRE(!vi->OnChar(event));
  }

  // Test navigate command keys.
  for (const auto& nav_key : std::vector<int> {
    WXK_BACK,WXK_DELETE,WXK_RETURN,WXK_LEFT,WXK_DOWN,WXK_UP,WXK_RIGHT,
    WXK_PAGEUP,WXK_PAGEDOWN,WXK_TAB})
  {
    event.m_keyCode = nav_key;
    REQUIRE(!vi->OnKeyDown(event));
    ChangeMode( vi, ESC, wxExVi::MODE_NORMAL);
  }
  event.m_keyCode = WXK_NONE;
  REQUIRE( vi->OnKeyDown(event));

  // Test navigate with [ and ].
  event.m_uniChar = '[';
  REQUIRE(!vi->OnChar(event));
  event.m_uniChar= ']';
  REQUIRE(!vi->OnChar(event));
  vi->GetSTC()->AppendText("{}");
  event.m_uniChar = '[';
  REQUIRE(!vi->OnChar(event));
  event.m_uniChar= ']';
  REQUIRE(!vi->OnChar(event));
  
  // Test insert command.
  stc->SetText("aaaaa");
  REQUIRE( vi->GetMode() == wxExVi::MODE_NORMAL);
  REQUIRE( vi->Command("i"));
  REQUIRE( vi->GetMode() == wxExVi::MODE_INSERT);
  REQUIRE( vi->GetLastCommand() == "i");
  REQUIRE( vi->Command("xxxxxxxx"));
  REQUIRE( stc->GetText().Contains("xxxxxxxx"));
  REQUIRE( vi->GetLastCommand() == "i");
  ChangeMode( vi, ESC, wxExVi::MODE_NORMAL);
  REQUIRE( vi->GetRegisterInsert() == "xxxxxxxx");
  REQUIRE( vi->GetLastCommand() == wxString("ixxxxxxxx") + ESC);
  for (int i = 0; i < 10; i++)
    REQUIRE( vi->Command("."));
  REQUIRE( stc->GetText().Contains("xxxxxxxxxxxxxxxxxxxxxxxxxxx"));
  
  // Test MODE_INSERT commands.
  std::vector<std::string> commands;
  for (auto& it1 : vi->GetInsertCommands())
  {
    ChangeMode( vi, std::string(1, it1.first), wxExVi::MODE_INSERT);
    ChangeMode( vi, ESC, wxExVi::MODE_NORMAL);
    commands.push_back(std::string(1, it1.first));
  }

  // Test MODE_INSERT commands and delete command on readonly document.
  commands.insert(commands.end(), {"dd", "d0", "d$", "dw", "de"});
  stc->SetReadOnly(true);
  stc->EmptyUndoBuffer();
  stc->SetSavePoint();
  for (auto& it2 : commands)
  {
    INFO( it2);
    REQUIRE( vi->Command(it2));
    REQUIRE( vi->GetMode() == wxExVi::MODE_NORMAL);
    REQUIRE(!stc->GetModify());
  }

  // Test MODE_INSERT commands on hexmode document.
  stc->SetReadOnly(false);
  stc->Reload(wxExSTC::STC_WIN_HEX);
  REQUIRE( stc->HexMode());
  REQUIRE(!stc->GetModify());
  for (auto& it3 : commands)
  {
    REQUIRE( vi->Command(it3) );
  }
  REQUIRE( vi->GetMode() == wxExVi::MODE_NORMAL);
  REQUIRE(!stc->GetModify());
  stc->Reload();
  REQUIRE(!stc->HexMode());
  REQUIRE(!stc->GetModify());
  stc->SetReadOnly(false);

  // Test insert command.  
  ChangeMode( vi, "i", wxExVi::MODE_INSERT);
  ChangeMode( vi, ESC, wxExVi::MODE_NORMAL);
  ChangeMode( vi, "iyyyyy", wxExVi::MODE_INSERT);
  REQUIRE( stc->GetText().Contains("yyyyy"));
  REQUIRE(!stc->GetText().Contains("iyyyyy"));
  ChangeMode( vi, ESC, wxExVi::MODE_NORMAL);
  const wxString lastcmd = wxString("iyyyyy") + ESC;
  REQUIRE( vi->GetLastCommand() == lastcmd);
  REQUIRE( vi->GetInsertedText() == "yyyyy");
  REQUIRE( vi->Command("."));
  REQUIRE( vi->GetLastCommand() == lastcmd);
  REQUIRE(!vi->Command(";"));
  REQUIRE( vi->GetLastCommand() == lastcmd);
  REQUIRE( vi->Command("ma"));
  REQUIRE( vi->GetLastCommand() == lastcmd);
  REQUIRE( vi->Command("100izz"));
  REQUIRE( vi->GetMode() == wxExVi::MODE_INSERT);
  REQUIRE(!stc->GetText().Contains("izz"));
  ChangeMode( vi, ESC, wxExVi::MODE_NORMAL);
  REQUIRE( stc->GetText().Contains(wxString('z', 200)));

  // Test abbreviate.
  for (auto& abbrev : GetAbbreviations())
  {
    REQUIRE( vi->Command(":ab " + abbrev.first + " " + abbrev.second));
    REQUIRE( vi->Command("iabbreviation " + abbrev.first + " "));
    ChangeMode( vi, ESC, wxExVi::MODE_NORMAL);
    REQUIRE( vi->GetInsertedText() == "abbreviation "  + abbrev.first + " ");
    REQUIRE( stc->GetText().Contains("abbreviation " + abbrev.second));
    REQUIRE(!stc->GetText().Contains(abbrev.first));
  }

  // Test substitute command.
  stc->SetText("999");
  REQUIRE( vi->Command(":1,$s/xx/yy/g")); // so &, ~ are ok
  REQUIRE( vi->Command("i"));
  REQUIRE( vi->GetMode() == wxExVi::MODE_INSERT);
  REQUIRE( vi->Command("b"));
  ChangeMode( vi, ESC, wxExVi::MODE_NORMAL);
  REQUIRE( stc->GetText().Contains("b"));
  stc->SetText("xxxxxxxxxx\nxxxxxxxx\naaaaaaaaaa\n");
  REQUIRE( vi->Command(":.="));
  REQUIRE( vi->Command(":1,$s/xx/yy/g"));
  REQUIRE(!stc->GetText().Contains("xx"));
  REQUIRE( stc->GetText().Contains("yy"));
  REQUIRE( vi->GetLastCommand() == ":1,$s/xx/yy/g");
  
  // Test change command.
  stc->SetText("xxxxxxxxxx second\nxxxxxxxx\naaaaaaaaaa\n");
  REQUIRE( stc->GetLineCount() == 4);
  REQUIRE( vi->Command(":1"));
  REQUIRE( vi->Command("cc"));
  REQUIRE( vi->GetMode() == wxExVi::MODE_INSERT);
  REQUIRE( vi->Command("zzz"));
  ChangeMode( vi, ESC, wxExVi::MODE_NORMAL);
  REQUIRE( stc->GetLineCount() == 4);
  REQUIRE( stc->GetLineText(0) == "zzz");
  stc->SetText("xxxxxxxxxx second\nxxxxxxxx\naaaaaaaaaa\n");
  REQUIRE( vi->Command(":1"));
  REQUIRE( vi->Command("ce"));
  REQUIRE( vi->GetMode() == wxExVi::MODE_INSERT);
  REQUIRE( vi->Command("zzz"));
  ChangeMode( vi, ESC, wxExVi::MODE_NORMAL);
  REQUIRE( stc->GetLineCount() == 4);
  REQUIRE( stc->GetLineText(0) == "zzz second");
  stc->SetText("xxxxxxxxxx second third\nxxxxxxxx\naaaaaaaaaa\n");
  REQUIRE( vi->Command(":1"));
  REQUIRE( vi->Command("2ce"));
  REQUIRE( vi->GetMode() == wxExVi::MODE_INSERT);
  REQUIRE( vi->Command("zzz"));
  ChangeMode( vi, ESC, wxExVi::MODE_NORMAL);
  REQUIRE( stc->GetLineCount() == 4);
  REQUIRE( stc->GetLineText(0) == "zzz third");

  // Test special delete commands.
  for (auto& delete_command : std::vector<std::string> {"dgg","3dw", "d3w"})
  {
    REQUIRE( vi->Command(delete_command));
    REQUIRE( vi->GetLastCommand() == delete_command);
  }

  // Test yank commands.
  stc->SetText("xxxxxxxxxx second\nxxxxxxxx\naaaaaaaaaa\n");
  REQUIRE( vi->Command(":1"));
  REQUIRE( vi->Command("yw"));
  REQUIRE( vi->GetSelectedText() == "xxxxxxxxxx ");
  REQUIRE( vi->Command("w"));
  REQUIRE( vi->Command("x"));
  REQUIRE( stc->GetText().Contains("second"));
  REQUIRE( vi->Command("p"));
  REQUIRE( stc->GetText().Contains("second"));
  
  // Test motion commands: navigate, yank, delete, and change.
  wxExFindReplaceData::Get()->SetFindString("xx");
  for (auto& motion_command : vi->GetMotionCommands())
  {
    if (motion_command.first <= 255)
    {
      stc->SetText("xxxxxxxxxx\nyyyyyyyyyy\nzzzzzzzzzz\nftFT\n{section}");

      // test navigate
      std::string nc(
        motion_command.first == 'f' || motion_command.first == 't' ||
        motion_command.first == 'F' || motion_command.first == 'T' ||
        motion_command.first == '\'' ? 2: 1, motion_command.first);
      INFO( nc );
      REQUIRE( vi->Command(nc));
      
      // test navigate while in rect mode
      ChangeMode( vi, "K", wxExVi::MODE_VISUAL_RECT);
      REQUIRE( vi->Command( nc ));
      ChangeMode( vi, ESC, wxExVi::MODE_NORMAL);
  
      // test yank
      std::string mc(
        motion_command.first == 'f' || motion_command.first == 't' ||
        motion_command.first == 'F' || motion_command.first == 'T' ||
        motion_command.first == '\'' ? 3: 2, 'y');
      mc[0] = 'y';
      mc[1] = motion_command.first;
      INFO( mc);
      REQUIRE( vi->Command(mc));
      REQUIRE( vi->GetLastCommand() == mc);
      REQUIRE( vi->GetMode() == wxExVi::MODE_NORMAL);

      // test delete
      mc[0] = 'd';
      INFO( mc);
      REQUIRE( vi->Command(mc));
      REQUIRE( vi->GetLastCommand() == mc);
      
      // test change
      mc[0] = 'c';
      INFO( mc);
      REQUIRE( vi->Command(mc));
      REQUIRE( vi->GetLastCommand() == mc);
      ChangeMode( vi, ESC, wxExVi::MODE_NORMAL);
    }
  }

  // Test other commands.
  stc->SetText("xxxxxxxxxx second\nxxxxxxxx\naaaaaaaaaa\n");
  for (auto& other_command : std::vector<std::string> {
    "gg","zc","zo","zE",">>","<<"})
  {
    REQUIRE( vi->Command(other_command));
  }

  // Special put test. 
  // Put should not put text within a line, but after it, or before it.
  stc->SetText("the chances of anything coming from mars\n");
  vi->Command("$");
  vi->Command("h");
  vi->Command("h");
  vi->Command("yy");
  REQUIRE( 
    wxString(stc->GetVi().GetMacros().GetRegister('0')).Contains("the chances of anything"));
  vi->Command("p");
  vi->Command("P");
  REQUIRE( stc->GetText().Contains(
    "the chances of anything coming from mars"));
  REQUIRE(!stc->GetText().Contains("mathe"));

  // Test calc.
  stc->SetText("this text contains xx");
  vi->Command("i");
  vi->Command("=5+5");
  vi->Command("");
  REQUIRE( stc->GetText().Contains("10"));
  
  // Test macro.
  // First load macros.
  REQUIRE( wxExViMacros::LoadDocument());
  stc->SetText("this text contains xx");
  for (const auto& macro : std::vector< std::vector< std::string> > {
    {"10w"},
    {"dw"},
    {"de"},
    {"yw"},
    {"yk"},
    {"/xx","rz"}})  
  {
    REQUIRE( vi->Command("qt"));
    
    std::string all;
    
    for (auto& command : macro)
    {
      REQUIRE( vi->Command(command));
      all += command;
    }
    
    REQUIRE( vi->Command("q"));
    REQUIRE( stc->GetVi().GetMacros().GetRegister('t') == all);
  }
  
  REQUIRE( vi->Command("@t"));
  REQUIRE( vi->Command("@@"));
  REQUIRE( vi->Command("."));
  REQUIRE( vi->Command("10@t"));

  // Next should be OK, but crashes due to input expand variable.
  //REQUIRE( vi->Command("@hdr@"));
  
  // Test variables.
  stc->SetText("");
  REQUIRE( vi->Command("@Date@"));
  REQUIRE(!stc->GetText().Contains("Date"));
  stc->SetText("");
  REQUIRE( vi->Command("@Year@"));
  REQUIRE( stc->GetText().Contains("20"));
//  REQUIRE(!vi->Command("@xxx@"));
  REQUIRE( stc->SetLexer("cpp"));
  stc->SetText("");
  REQUIRE( vi->Command("@Cb@"));
  REQUIRE( vi->Command("@Ce@"));
  REQUIRE( stc->GetText().Contains("//"));
  stc->SetText("");
  REQUIRE( vi->Command("@Cl@"));
  REQUIRE( stc->GetText().Contains("//"));
  REQUIRE( vi->Command("@Nl@"));
  
  // Test illegal command.
  REQUIRE(!vi->Command("dx"));
  REQUIRE( vi->GetLastCommand() != "dx");
  REQUIRE(!vi->Command(":xxx"));
  ChangeMode( vi, ESC, wxExVi::MODE_NORMAL);

  // Test registers
  stc = new wxExSTC(GetFrame(), GetTestFile());
  AddPane(GetFrame(), stc);
  vi = &stc->GetVi();
  const int ctrl_r = 18;
  REQUIRE( vi->Command("i"));
  REQUIRE( vi->Command(wxString(wxUniChar(ctrl_r)).ToStdString()));
  REQUIRE( vi->Command("_"));
  ChangeMode( vi, ESC, wxExVi::MODE_NORMAL);
  
  REQUIRE( vi->Command("i"));
  REQUIRE( vi->Command(wxString(wxUniChar(ctrl_r)).ToStdString()));
  REQUIRE( vi->Command("%"));
  ChangeMode( vi, ESC, wxExVi::MODE_NORMAL);
  REQUIRE( stc->GetText().Contains("test.h"));
  
  REQUIRE( vi->Command("yy"));
  stc->SetText("");
  REQUIRE( vi->Command("i"));
  REQUIRE( vi->Command(wxString(wxUniChar(ctrl_r)).ToStdString()));
  REQUIRE( vi->Command("0"));
  ChangeMode( vi, ESC, wxExVi::MODE_NORMAL);
  REQUIRE( stc->GetText().Contains("test.h"));
  
  stc->SetText("XXXXX");
  REQUIRE( vi->Command("dd"));
  REQUIRE( vi->Command("i"));
  REQUIRE( vi->Command(wxString(wxUniChar(ctrl_r)).ToStdString()));
  REQUIRE( vi->Command("1"));
  ChangeMode( vi, ESC, wxExVi::MODE_NORMAL);
  REQUIRE( stc->GetText().Contains("XXXXX"));
  
  stc->SetText("YYYYY");
  REQUIRE( vi->Command("dd"));
  REQUIRE( vi->Command("i"));
  REQUIRE( vi->Command(wxString(wxUniChar(ctrl_r)).ToStdString()));
  REQUIRE( vi->Command("2"));
  ChangeMode( vi, ESC, wxExVi::MODE_NORMAL);
  REQUIRE( stc->GetText().Contains("XXXXX"));
  
  // Test visual modes.
  for (const auto& visual : std::vector<std::pair<std::string, int>> {
    {"v",wxExVi::MODE_VISUAL},
    {"V",wxExVi::MODE_VISUAL_LINE},
    {"K",wxExVi::MODE_VISUAL_RECT}})
  {
    ChangeMode( vi, visual.first, visual.second);
    ChangeMode( vi, "jjj", visual.second);
    ChangeMode( vi, ESC, wxExVi::MODE_NORMAL);
    
    event.m_uniChar = visual.first[0];
    REQUIRE(!vi->OnChar(event));
    REQUIRE( vi->GetMode() == visual.second);
    ChangeMode( vi, ESC, wxExVi::MODE_NORMAL);
  }
  
  // Test goto, /, ?, n and N.
  stc->SetText("aaaaa\nbbbbb\nccccc\naaaaa\ne\nf\ng\nh\ni\nj\nk\n");
  REQUIRE( stc->GetLineCount() == 12);
  stc->GotoLine(2);
  for (const auto& go : std::vector<std::pair<std::string, int>> {
    {"gg",0},
    {"G",11},
    {"1G",11},
    {"10G",9},
    {"10000G",11},
    {":$",11},
    {":100",11},
    {"/bbbbb",1},
    {"/d",1},
    {"/a",3},
    {"n",3},
    {"N",3},
    {"?bbbbb",1},
    {"?d",1},
    {"?a",0},
    {"n",0},
    {"N",0}}) {
    if (go.first.back() != 'd')
      REQUIRE( vi->Command(go.first));
    else
      REQUIRE(!vi->Command(go.first));

    INFO( go.first);
    REQUIRE( stc->GetCurrentLine() == go.second);
  }
}
