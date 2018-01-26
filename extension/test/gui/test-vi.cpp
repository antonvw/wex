////////////////////////////////////////////////////////////////////////////////
// Name:      test-vi.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/vi.h>
#include <wx/extension/frd.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/vi-macros.h>
#include <wx/extension/vi-macros-mode.h>
#include <wx/extension/stc.h>
#include "test.h"

#define ESC "\x1b"

void ChangeMode(wxExVi* vi, const std::string& command, wxExViModes mode)
{
  REQUIRE( vi->Command(command));
  REQUIRE( vi->Mode().Get() == mode);
}

TEST_CASE("wxExVi")
{
  wxExSTC* stc = new wxExSTC(
    std::string("// vi: set ts=120 "
                "// this is a modeline"));
  AddPane(GetFrame(), stc);
  wxExVi* vi = &stc->GetVi();
  wxKeyEvent event(wxEVT_CHAR);
  
  // Test for modeline support.
  REQUIRE(stc->GetTabWidth() == 120);
  REQUIRE( vi->GetIsActive());
  REQUIRE( vi->Mode().Normal());

  // Test WXK_NONE.
  event.m_uniChar = WXK_NONE;
  REQUIRE( vi->OnChar(event));
  
  // First i enters insert mode, so is handled by vi, not to be skipped.
  event.m_uniChar = 'i';
  REQUIRE(!vi->OnChar(event));
  REQUIRE( vi->Mode().Insert());
  REQUIRE( vi->Mode().String() == "insert");
  // Second i (and more) all handled by vi.
  for (int i = 0; i < 10; i++) REQUIRE(!vi->OnChar(event));

  // Test control keys.
  for (const auto& control_key : std::vector<int> {
    WXK_CONTROL_B,WXK_CONTROL_E,WXK_CONTROL_F,WXK_CONTROL_G,
    WXK_CONTROL_J,WXK_CONTROL_P,WXK_CONTROL_Q})
  {
    event.m_uniChar = control_key;
    REQUIRE( vi->OnKeyDown(event));
    REQUIRE(!vi->OnChar(event));
  }
  
  // Test change number.
  event.m_uniChar = WXK_CONTROL_E;
  for (const auto& number: std::vector<std::string> {
    "101", "0xf7", "077", "-99"})
  {
    stc->SetText("number: " + number);
    vi->Command("gg");
    vi->Command("2w");
    REQUIRE( vi->OnKeyDown(event));
    REQUIRE(!vi->OnChar(event));
    CAPTURE( number);
//    REQUIRE(!stc->GetText().Contains(number));
  }
  
  // Test navigate command keys.
  for (const auto& nav_key : std::vector<int> {
    WXK_BACK, WXK_DELETE, WXK_RETURN, WXK_LEFT, WXK_DOWN, WXK_UP, WXK_RIGHT,
    WXK_PAGEUP, WXK_PAGEDOWN, WXK_TAB})
  {
    event.m_keyCode = nav_key;
    CAPTURE( nav_key);
//    REQUIRE(!vi->OnKeyDown(event));
    ChangeMode( vi, ESC, wxExViModes::NORMAL);
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
  REQUIRE( vi->Mode().Normal());
  REQUIRE( vi->Command("i"));
  REQUIRE( vi->Mode().Insert());
  REQUIRE( vi->GetLastCommand() == "i");
  REQUIRE( vi->Command("xxxxxxxx"));
  REQUIRE( stc->GetText().Contains("xxxxxxxx"));
  REQUIRE( vi->GetLastCommand() == "i");
  ChangeMode( vi, ESC, wxExViModes::NORMAL);
  REQUIRE( vi->GetRegisterInsert() == "xxxxxxxx");
  REQUIRE( vi->GetLastCommand() == wxString("ixxxxxxxx") + ESC);
  for (int i = 0; i < 10; i++)
    REQUIRE( vi->Command("."));
  REQUIRE( stc->GetText().Contains("xxxxxxxxxxxxxxxxxxxxxxxxxxx"));
  
  // Test insert commands.
  std::vector<std::string> commands;
  for (auto& it1 : vi->Mode().GetInsertCommands())
  {
    if (it1.first != 'c')
    {
      CAPTURE( it1.first);
      ChangeMode( vi, std::string(1, it1.first), wxExViModes::INSERT);
      ChangeMode( vi, ESC, wxExViModes::NORMAL);
      commands.push_back(std::string(1, it1.first));
    }
  }

  // Test insert commands and delete commands on readonly document.
  commands.insert(commands.end(), {"dd", "d0", "d$", "dw", "de"});
  stc->SetReadOnly(true);
  stc->EmptyUndoBuffer();
  stc->SetSavePoint();
  for (auto& it2 : commands)
  {
    REQUIRE( vi->Command(it2));
    REQUIRE( vi->Mode().Normal());
    REQUIRE(!stc->GetModify());
  }

  // Test insert on hexmode document.
  // TODO: add real chars and test.
  stc->SetReadOnly(false);
  stc->GetHexMode().Set(true);
  REQUIRE( stc->HexMode());
  REQUIRE(!stc->GetModify());
  REQUIRE( vi->Command("a") );
  REQUIRE( vi->Mode().Insert());
  ChangeMode( vi, ESC, wxExViModes::NORMAL);
  REQUIRE(!stc->GetModify());
  stc->GetHexMode().Set(false);
  REQUIRE(!stc->HexMode());
  REQUIRE(!stc->GetModify());
  stc->SetReadOnly(false);

  // Test insert command.  
  ChangeMode( vi, "i", wxExViModes::INSERT);
  ChangeMode( vi, ESC, wxExViModes::NORMAL);
  ChangeMode( vi, "iyyyyy", wxExViModes::INSERT);
  REQUIRE( stc->GetText().Contains("yyyyy"));
  REQUIRE(!stc->GetText().Contains("iyyyyy"));
  ChangeMode( vi, ESC, wxExViModes::NORMAL);
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
  REQUIRE( vi->Mode().Insert());
  REQUIRE(!stc->GetText().Contains("izz"));
  ChangeMode( vi, ESC, wxExViModes::NORMAL);
  REQUIRE( stc->GetText().Contains(wxString('z', 200)));

  // Test insert \n.
  ChangeMode( vi, "i\n\n\n\n", wxExViModes::INSERT);
  REQUIRE( stc->GetText().Contains("\n"));
  REQUIRE(!stc->GetText().Contains("i"));
  ChangeMode( vi, ESC, wxExViModes::NORMAL);
  REQUIRE( vi->GetInsertedText() == "\n\n\n\n");
  
  stc->SetText("");
  event.m_uniChar = 'i';
  REQUIRE(!vi->OnChar(event));
  REQUIRE( vi->Mode().Insert());
  REQUIRE( vi->GetInsertedText().empty());
  REQUIRE( vi->Mode().Insert());
  event.m_uniChar = WXK_RETURN;
  REQUIRE( vi->OnKeyDown(event));
  REQUIRE(!vi->OnChar(event));
  ChangeMode( vi, ESC, wxExViModes::NORMAL);
#ifndef __WXOSX__  
  REQUIRE( wxString(vi->GetInsertedText()).Contains("\n"));
#endif
  
  // Test abbreviate.
  for (auto& abbrev : GetAbbreviations())
  {
    REQUIRE( vi->Command(":ab " + abbrev.first + " " + abbrev.second));
    REQUIRE( vi->Command("iabbreviation " + abbrev.first + " "));
    ChangeMode( vi, ESC, wxExViModes::NORMAL);
    REQUIRE( vi->GetInsertedText() == "abbreviation "  + abbrev.first + " ");
    REQUIRE( stc->GetText().Contains("abbreviation " + abbrev.second));
    REQUIRE(!stc->GetText().Contains(abbrev.first));
  }
 
  // Test maps.
  stc->SetText("this text is not needed");
  vi->Command(":map 7 :%d");
  int ctrl_g = 7;
  vi->Command(std::string(1, ctrl_g));
  REQUIRE(stc->GetText().empty());

  // Test substitute command.
  stc->SetText("999");
  REQUIRE( vi->Command(":1,$s/xx/yy/g")); // so &, ~ are ok
  REQUIRE( vi->Command("i"));
  REQUIRE( vi->Mode().Insert());
  REQUIRE( vi->Command("b"));
  ChangeMode( vi, ESC, wxExViModes::NORMAL);
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
  REQUIRE( vi->Command("cw"));
  REQUIRE( vi->Mode().Insert());
  REQUIRE( vi->Command("zzz"));
  ChangeMode( vi, ESC, wxExViModes::NORMAL);
  REQUIRE( stc->GetLineCount() == 4);
  REQUIRE( stc->GetLineText(0) == "zzzsecond");
  stc->SetText("xxxxxxxxxx second\nxxxxxxxx\naaaaaaaaaa\n");
  REQUIRE( vi->Command(":1"));
  REQUIRE( vi->Command("ce"));
  REQUIRE( vi->Mode().Insert());
  REQUIRE( vi->Command("zzz"));
  ChangeMode( vi, ESC, wxExViModes::NORMAL);
  REQUIRE( stc->GetLineCount() == 4);
  REQUIRE( stc->GetLineText(0) == "zzz second");
  stc->SetText("xxxxxxxxxx second third\nxxxxxxxx\naaaaaaaaaa\n");
  REQUIRE( vi->Command(":1"));
  REQUIRE( vi->Command("2ce"));
  REQUIRE( vi->Mode().Insert());
  REQUIRE( vi->Command("zzz"));
  ChangeMode( vi, ESC, wxExViModes::NORMAL);
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
  
  vi->ResetSearchFlags();

  // Test motion commands: navigate, yank, delete, and change.
  wxExFindReplaceData::Get()->SetFindString("xx");
  for (auto& motion_command : vi->GetMotionCommands())
  {
    for (auto c : motion_command.first)
    {
      stc->SetText("xxxxxxxxxx\nyyyyyyyyyy\n"
                   "zzzzzzzzzz\nftFT\n"
                   "{section}yyyy\n"
                   "{anothersection}{finalsection}");

      // test navigate
      std::string nc(1, c);

      if (c == 'f' || c == 't' || c == 'F' || c == 'T' || c == '\'')
      {
        nc += "f";
      }

      CAPTURE( motion_command.first);
      CAPTURE( nc);
      REQUIRE( vi->Command(nc));
      
      // test navigate while in rect mode
      ChangeMode( vi, "K", wxExViModes::VISUAL_RECT);
      REQUIRE( vi->Command( nc ));
      REQUIRE( vi->Mode().Visual());
      ChangeMode( vi, ESC, wxExViModes::NORMAL);
      REQUIRE( vi->Mode().Normal());
      
      // test yank
      std::string mc(
        c == 'f' || c == 't' ||
        c == 'F' || c == 'T' ||
        c == '\'' ? 3: 2, 'y');

      mc[0] = 'y';
      mc[1] = c;

      CAPTURE( mc);
      REQUIRE( vi->Command(mc));
      REQUIRE( vi->GetLastCommand() == mc);
      REQUIRE( vi->Mode().Normal());

      // test delete
      mc[0] = 'd';
      CAPTURE( mc);
      REQUIRE( vi->Command(mc));
      REQUIRE( vi->GetLastCommand() == mc);
      REQUIRE( vi->Mode().Normal());
      
      // test change
      mc[0] = 'c';
      CAPTURE( mc);
      REQUIRE( vi->Command(mc));
      REQUIRE( vi->GetLastCommand() == mc);
      ChangeMode( vi, ESC, wxExViModes::NORMAL);
      REQUIRE( vi->Mode().Normal());
    }
  }

  // Test find.
  stc->SetText("some text to find");
  REQUIRE( vi->Mode().Normal());
  REQUIRE( vi->Command("/find"));
  REQUIRE( vi->Mode().Normal());
  REQUIRE( vi->Command("yb"));
  REQUIRE( vi->Mode().Normal());
  REQUIRE(!vi->Command("/xfind"));
  // TODO: fix
  // REQUIRE( vi->Command("/"  + std::string(1, WXK_CONTROL_R) + "0"));

  // Test % navigate.
  stc->SetText("{a brace and a close brace}");
  REQUIRE( vi->Command("y%"));
  REQUIRE( stc->GetSelectedText().size() == 27);
  
  // Test delete navigate.
  stc->SetText("xyz");
  REQUIRE( vi->Command(std::string(1, WXK_DELETE)));
  REQUIRE( stc->GetText() == "yz");

  // Test record find.
  stc->SetText("abc\nuvw\nxyz\n");

  // Test recording.
  REQUIRE(!vi->GetMacros().Mode()->IsRecording());
  vi->Command("");
  REQUIRE( vi->Command("qa"));
  REQUIRE( vi->GetMacros().Mode()->IsRecording());
  REQUIRE( vi->Command("/abc"));
  REQUIRE( vi->Command("q"));
  REQUIRE(!vi->GetMacros().Mode()->IsRecording());
  REQUIRE( vi->Command("@a"));
  REQUIRE( vi->Command(" "));
  REQUIRE( vi->Command("qb"));
  REQUIRE( vi->GetMacros().Mode()->IsRecording());
  REQUIRE( vi->Command("?abc"));
  REQUIRE( vi->Command("q"));
  REQUIRE(!vi->GetMacros().Mode()->IsRecording());
  REQUIRE( vi->Command("@b"));
  REQUIRE( vi->Command(" "));

  // Test fold.
  for (auto& fold: std::vector<std::string> {"zo", "zc", "zE", "zf"})
  {
    REQUIRE( vi->Command(fold));
    REQUIRE( vi->GetLastCommand() == fold);
  }
  
  // Test other commands (ZZ not tested).
  for (auto& other_command : vi->GetOtherCommands())
  {
    stc->SetText("first second\nthird\nfourth\n");

    if (!isalpha(other_command.first.front()) && 
        other_command.first.front() != '\x12' &&
        other_command.first.front() != '@')
    {
      for (auto c : other_command.first)
      {
        // prevent wxExBrowserSearch (for travis)
        if (c == 'T')
        {
          continue;
        }

        if (c != '\t')
          REQUIRE( vi->Command(std::string(1, c)));
        else
          REQUIRE(!vi->Command(std::string(1, c)));
      }
    }
    else
    {
      const std::string oc(
        other_command.first == "m" || 
        other_command.first == "q" || 
        other_command.first == "r" || 
        other_command.first == "\x12" || 
        other_command.first == "@" ?
          other_command.first + "a": other_command.first);
      CAPTURE( oc );

      if (oc != "z")
        REQUIRE( vi->Command(oc));
      else if (oc != "qa")
        REQUIRE(!vi->Command(oc));
    }
  }

  if (vi->GetMacros().Mode()->IsRecording())
  {
    vi->Command("q");
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
  ChangeMode( vi, ESC, wxExViModes::NORMAL);
  vi->Command("=5+5");
  REQUIRE( wxString(stc->GetVi().GetMacros().GetRegister('0')).Contains("10"));
  vi->Command("=5+5+5");
  REQUIRE( wxString(stc->GetVi().GetMacros().GetRegister('0')).Contains("15"));

  // Test macro.
  // First load macros.
  REQUIRE( wxExViMacros::LoadDocument());
  for (const auto& macro : std::vector< std::vector< std::string> > {
    {"10w"},
    {"dw"},
    {"de"},
    {"yw"},
    {"yk"},
    {"/xx","rz"}})  
  {
    stc->SetText("this text contains xx");
    
    REQUIRE( vi->Command("qt"));
    REQUIRE( vi->GetMacros().Mode()->IsRecording());
    
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
  REQUIRE( stc->GetLexer().Set("cpp"));
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
  ChangeMode( vi, ESC, wxExViModes::NORMAL);

  // Test registers
  stc->GetFile().FileNew("test.h");
  stc->SetText("");
  const std::string ctrl_r = "\x12";
  REQUIRE( vi->Command("i"));
  REQUIRE( vi->Command(ctrl_r + "_"));
  ChangeMode( vi, ESC, wxExViModes::NORMAL);
  
  stc->SetText("");
  REQUIRE( vi->Command("i"));
  REQUIRE( vi->Command(ctrl_r + "%"));
  ChangeMode( vi, ESC, wxExViModes::NORMAL);
  REQUIRE( stc->GetText() == "test.h");
  
  REQUIRE( vi->Command("yy"));
  stc->SetText("");
  REQUIRE( vi->Command("i"));
  REQUIRE( vi->Command(ctrl_r + "0"));
  ChangeMode( vi, ESC, wxExViModes::NORMAL);
  REQUIRE( stc->GetText() == "test.h");
  
  stc->SetText("XXXXX");
  REQUIRE( vi->Command("dd"));
  REQUIRE( vi->Command("i"));
  REQUIRE( vi->Command(ctrl_r + "1"));
  ChangeMode( vi, ESC, wxExViModes::NORMAL);
  REQUIRE( stc->GetText() == "XXXXX");
  
  stc->SetText("YYYYY");
  REQUIRE( vi->Command("dd"));
  REQUIRE( vi->Command("i"));
  REQUIRE( vi->Command(ctrl_r + "2"));
  REQUIRE( vi->Command("2"));
  ChangeMode( vi, ESC, wxExViModes::NORMAL);
  REQUIRE( stc->GetText().Contains("XXXXX"));
  
  // Test visual modes.
  for (const auto& visual : std::vector<std::pair<std::string, wxExViModes>> {
    {"v",wxExViModes::VISUAL},
    {"V",wxExViModes::VISUAL_LINE},
    {"K",wxExViModes::VISUAL_RECT}})
  {
    ChangeMode( vi, visual.first, visual.second);
    ChangeMode( vi, "jjj", visual.second);
    ChangeMode( vi, visual.first, visual.second); // second has no effect
    // enter illegal command
    vi->Command("g");
    vi->Command("j");
    ChangeMode( vi, ESC, wxExViModes::NORMAL);
    
    event.m_uniChar = visual.first[0];
    REQUIRE(!vi->OnChar(event));
    REQUIRE( vi->Mode().Get() == visual.second);
    ChangeMode( vi, ESC, wxExViModes::NORMAL);
  }
  
  stc->SetText("123456789");
  vi->Command("v");
  REQUIRE( vi->Mode().Visual());
  vi->VisualExtend(0, 10);
  REQUIRE( vi->GetSelectedText() == "123456789");
  vi->Mode().Escape();

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
    
    if (go.first[0] == '/' || go.first[0] == '?')
    {
      // A / or ? should not set a last command.
      REQUIRE( vi->GetLastCommand()[0] != go.first[0]);
    }

    REQUIRE( stc->GetCurrentLine() == go.second);
  }
  
  // test ctags
  stc->SetText("no tag");
  REQUIRE( vi->Command("Q"));
  stc->SetText("wxExTestApp");
  REQUIRE( vi->Command("Q"));
  REQUIRE( vi->Command("S"));
}
