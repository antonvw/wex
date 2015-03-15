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

void ChangeMode(wxExVi* vi, const std::string& command, int mode)
{
  CPPUNIT_ASSERT( vi->Command(command));
  CPPUNIT_ASSERT( vi->GetMode() == mode);
}

void wxExGuiTestFixture::testVi()
{
  // Test for modeline support.
  wxExSTC* stc = new wxExSTC(m_Frame, 
    "// vi: set ts=120 "
    "// this is a modeline");
    
  wxExVi* vi = &stc->GetVi();
  
  CPPUNIT_ASSERT(stc->GetTabWidth() == 120);
  CPPUNIT_ASSERT( vi->GetIsActive());
  CPPUNIT_ASSERT( vi->ModeNormal());

  wxKeyEvent event(wxEVT_CHAR);
  
  // Test WXK_NONE.
  event.m_uniChar = WXK_NONE;
  CPPUNIT_ASSERT( vi->OnChar(event));
  
  // First i enters insert mode, so is handled by vi, not to be skipped.
  event.m_uniChar = 'i';
  CPPUNIT_ASSERT(!vi->OnChar(event));
  CPPUNIT_ASSERT( vi->GetMode() == wxExVi::MODE_INSERT);
  
  // Second i (and more) all handled by vi.
  for (int i = 0; i < 10; i++) CPPUNIT_ASSERT(!vi->OnChar(event));

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
  ChangeMode( vi, ESC, wxExVi::MODE_NORMAL);
  vi->GetMacros().StopRecording();
  CPPUNIT_ASSERT(!vi->GetMacros().IsRecording());
  CPPUNIT_ASSERT( vi->GetMacros().IsRecorded("a"));
  CPPUNIT_ASSERT(!vi->GetMacros().IsRecorded("b"));
  CPPUNIT_ASSERT( vi->MacroPlayback("a"));
  ChangeMode( vi, ESC, wxExVi::MODE_NORMAL);

  // Vi control key tests.
  for (auto& control_key : std::vector<int> {
    WXK_CONTROL_B,WXK_CONTROL_E,WXK_CONTROL_F,WXK_CONTROL_G,
    WXK_CONTROL_J,WXK_CONTROL_P,WXK_CONTROL_Q})
  {
    event.m_uniChar = control_key;
    CPPUNIT_ASSERT( vi->OnKeyDown(event));
    CPPUNIT_ASSERT(!vi->OnChar(event));
  }

  // Vi navigation command tests.
  for (auto& nav_key : std::vector<int> {
    WXK_BACK,WXK_DELETE,WXK_RETURN,WXK_LEFT,WXK_DOWN,WXK_UP,WXK_RIGHT,
    WXK_PAGEUP,WXK_PAGEDOWN,WXK_TAB})
  {
    event.m_keyCode = nav_key;
    CPPUNIT_ASSERT(!vi->OnKeyDown(event));
    ChangeMode( vi, ESC, wxExVi::MODE_NORMAL);
  }

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
  ChangeMode( vi, ESC, wxExVi::MODE_NORMAL);
  CPPUNIT_ASSERT( stc->GetText().Contains("xxxxxxxx"));
  CPPUNIT_ASSERT( vi->GetRegisterInsert() == "xxxxxxxx");
  CPPUNIT_ASSERT( vi->GetLastCommand() == wxString("ixxxxxxxx") + ESC);
  
  for (int i = 0; i < 10; i++)
    CPPUNIT_ASSERT( vi->Command("."));
    
  CPPUNIT_ASSERT( stc->GetText().Contains("xxxxxxxxxxxxxxxxxxxxxxxxxxx"));
  
  // Test MODE_INSERT commands.
  std::vector<std::string> commands {"a", "i", "o", "A", "C", "I", "O", "R"};
  
  for (auto& it1 : commands)
  {
    ChangeMode( vi, it1, wxExVi::MODE_INSERT);
    ChangeMode( vi, ESC, wxExVi::MODE_NORMAL);
  }

  // Test MODE_INSERT commands and delete command on readonly document.
  commands.insert(commands.end(), {"dd", "d0", "d$", "dw", "de"});
  
  stc->SetReadOnly(true);
  stc->EmptyUndoBuffer();
  stc->SetSavePoint();
  
  for (auto& it2 : commands)
  {
    CPPUNIT_ASSERT( vi->Command(it2));
    CPPUNIT_ASSERT( vi->GetMode() == wxExVi::MODE_NORMAL);
    CPPUNIT_ASSERT(!stc->GetModify());
  }

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
  ChangeMode( vi, "i", wxExVi::MODE_INSERT);
  ChangeMode( vi, ESC, wxExVi::MODE_NORMAL);

  ChangeMode( vi, "iyyyyy", wxExVi::MODE_INSERT);
  CPPUNIT_ASSERT( stc->GetText().Contains("yyyyy"));
  CPPUNIT_ASSERT(!stc->GetText().Contains("iyyyyy"));
  ChangeMode( vi, ESC, wxExVi::MODE_NORMAL);

  const wxString lastcmd = wxString("iyyyyy") + ESC;

  CPPUNIT_ASSERT( vi->GetLastCommand() == lastcmd);
  CPPUNIT_ASSERT( vi->GetInsertedText() == "yyyyy");
  CPPUNIT_ASSERT( vi->Command("."));
  CPPUNIT_ASSERT( vi->GetLastCommand() == lastcmd);
  CPPUNIT_ASSERT(!vi->Command(";"));
  CPPUNIT_ASSERT( vi->GetLastCommand() == lastcmd);
  CPPUNIT_ASSERT( vi->Command("ma"));
  CPPUNIT_ASSERT( vi->GetLastCommand() == lastcmd);
  
  CPPUNIT_ASSERT( vi->Command("100izz"));
  CPPUNIT_ASSERT( vi->GetMode() == wxExVi::MODE_INSERT);
  CPPUNIT_ASSERT(!stc->GetText().Contains("izz"));
  ChangeMode( vi, ESC, wxExVi::MODE_NORMAL);
  CPPUNIT_ASSERT( stc->GetText().Contains(wxString('z', 200)));
  
  for (auto& abbrev : m_Abbreviations)
  {
    CPPUNIT_ASSERT( vi->Command(":ab " + abbrev.first + " " + abbrev.second));
    CPPUNIT_ASSERT( vi->Command("iabbreviation " + abbrev.first + " "));
    ChangeMode( vi, ESC, wxExVi::MODE_NORMAL);
    CPPUNIT_ASSERT( vi->GetInsertedText() == "abbreviation "  + abbrev.first + " ");
    CPPUNIT_ASSERT( stc->GetText().Contains("abbreviation " + abbrev.second));
    CPPUNIT_ASSERT(!stc->GetText().Contains(abbrev.first));
  }

  stc->SetText("999");
  CPPUNIT_ASSERT( vi->Command("i"));
  CPPUNIT_ASSERT( vi->GetMode() == wxExVi::MODE_INSERT);
  CPPUNIT_ASSERT( vi->Command("b"));
  ChangeMode( vi, ESC, wxExVi::MODE_NORMAL);
  CPPUNIT_ASSERT( stc->GetText().Contains("b"));

  // Test commands that do not change mode, and all return true.
  commands.clear();
  commands.insert(commands.end(), {
    "b","e","h","j","k","l"," ","p","u","w","x",
    "B","D","E","G","H","J","L","M","P","W","X",
    "^","~","$","{","}","(",")","%","*","#"});

  for (auto& it4 : commands)
  {
    ChangeMode( vi, it4, wxExVi::MODE_NORMAL);
  }

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
  ChangeMode( vi, ESC, wxExVi::MODE_NORMAL);
  CPPUNIT_ASSERT( stc->GetLineCount() == 4);
  CPPUNIT_ASSERT( stc->GetLineText(0) == "zzz");
  
  stc->SetText("xxxxxxxxxx second\nxxxxxxxx\naaaaaaaaaa\n");
  CPPUNIT_ASSERT( vi->Command(":1"));
  CPPUNIT_ASSERT( vi->Command("cw"));
  CPPUNIT_ASSERT( vi->GetMode() == wxExVi::MODE_INSERT);
  CPPUNIT_ASSERT( vi->Command("zzz"));
  ChangeMode( vi, ESC, wxExVi::MODE_NORMAL);
  CPPUNIT_ASSERT( stc->GetLineCount() == 4);
  CPPUNIT_ASSERT( stc->GetLineText(0) == "zzz second");

  // Test delete commands.
  for (auto& delete_command : std::vector<std::string> {
    "dd","de","dh","dj","dk","dl","dw","dG","d0","d$","dgg","3dw"})
  {
    CPPUNIT_ASSERT( vi->Command(delete_command));
    CPPUNIT_ASSERT( vi->GetLastCommand() == delete_command);
  }

  // Test yank commands.
  stc->SetText("xxxxxxxxxx second\nxxxxxxxx\naaaaaaaaaa\n");
  CPPUNIT_ASSERT( vi->Command(":1"));
  CPPUNIT_ASSERT( vi->Command("yw"));
  CPPUNIT_ASSERT( vi->GetSelectedText() == "xxxxxxxxxx ");
  
  CPPUNIT_ASSERT( vi->Command("w"));
  CPPUNIT_ASSERT( vi->Command("x"));
  CPPUNIT_ASSERT( stc->GetText().Contains("second"));
  CPPUNIT_ASSERT( vi->Command("p"));
  CPPUNIT_ASSERT( stc->GetText().Contains("second"));
  
  for (auto& yank_command : std::vector<std::string> {
    "ye","yh","yj","yk","yl","yw","yy","y0","y$","3yw"})
  {
    CPPUNIT_ASSERT( vi->Command(yank_command));
    CPPUNIT_ASSERT( vi->GetLastCommand() == yank_command);
  }

  // Test other commands.
  stc->SetText("xxxxxxxxxx second\nxxxxxxxx\naaaaaaaaaa\n");
    
  for (auto& other_command : std::vector<std::string> {
    "fx","Fx",";","gg","zc","zo","zE",">>","<<"})
  {
    CPPUNIT_ASSERT( vi->Command(other_command));
  }

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
  
  // Test variables.
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
  CPPUNIT_ASSERT(!vi->Command(":xxx"));
  ChangeMode( vi, ESC, wxExVi::MODE_NORMAL);

  // Test registers
  stc = new wxExSTC(m_Frame, GetTestFile());
  vi = &stc->GetVi();
  const int ctrl_r = 18;
  CPPUNIT_ASSERT( vi->Command("i"));
  CPPUNIT_ASSERT( vi->Command(wxString(wxUniChar(ctrl_r)).ToStdString()));
  CPPUNIT_ASSERT( vi->Command("_"));
  ChangeMode( vi, ESC, wxExVi::MODE_NORMAL);
  
  CPPUNIT_ASSERT( vi->Command("i"));
  CPPUNIT_ASSERT( vi->Command(wxString(wxUniChar(ctrl_r)).ToStdString()));
  CPPUNIT_ASSERT( vi->Command("%"));
  ChangeMode( vi, ESC, wxExVi::MODE_NORMAL);
  CPPUNIT_ASSERT( stc->GetText().Contains("test.h"));
  
  CPPUNIT_ASSERT( vi->Command("yy"));
  stc->SetText("");
  CPPUNIT_ASSERT( vi->Command("i"));
  CPPUNIT_ASSERT( vi->Command(wxString(wxUniChar(ctrl_r)).ToStdString()));
  CPPUNIT_ASSERT( vi->Command("0"));
  ChangeMode( vi, ESC, wxExVi::MODE_NORMAL);
  CPPUNIT_ASSERT( stc->GetText().Contains("test.h"));
  
  stc->SetText("XXXXX");
  CPPUNIT_ASSERT( vi->Command("dd"));
  CPPUNIT_ASSERT( vi->Command("i"));
  CPPUNIT_ASSERT( vi->Command(wxString(wxUniChar(ctrl_r)).ToStdString()));
  CPPUNIT_ASSERT( vi->Command("1"));
  ChangeMode( vi, ESC, wxExVi::MODE_NORMAL);
  CPPUNIT_ASSERT( stc->GetText().Contains("XXXXX"));
  
  stc->SetText("YYYYY");
  CPPUNIT_ASSERT( vi->Command("dd"));
  CPPUNIT_ASSERT( vi->Command("i"));
  CPPUNIT_ASSERT( vi->Command(wxString(wxUniChar(ctrl_r)).ToStdString()));
  CPPUNIT_ASSERT( vi->Command("2"));
  ChangeMode( vi, ESC, wxExVi::MODE_NORMAL);
  CPPUNIT_ASSERT( stc->GetText().Contains("XXXXX"));
  
  // Test visual modes.
  for (auto& visual : std::vector<std::pair<std::string, int>> {
    {"v",wxExVi::MODE_VISUAL},
    {"V",wxExVi::MODE_VISUAL_LINE},
    {"F",wxExVi::MODE_VISUAL_RECT}})
  {
    ChangeMode( vi, visual.first, visual.second);
    ChangeMode( vi, "jjj", visual.second);
    ChangeMode( vi, ESC, wxExVi::MODE_NORMAL);
    
    event.m_uniChar = visual.first[0];
    CPPUNIT_ASSERT(!vi->OnChar(event));
    CPPUNIT_ASSERT( vi->GetMode() == visual.second);
    ChangeMode( vi, ESC, wxExVi::MODE_NORMAL);
  }
  
  // Test goto, /, ?, n and N.
  stc->SetText("aaaaa\nbbbbb\nccccc\naaaaa\ne\nf\ng\nh\ni\nj\nk\n");
  CPPUNIT_ASSERT( stc->GetLineCount() == 12);
  stc->GotoLine(2);
  
  for (auto& go : std::vector<std::pair<std::string, int>> {
    {"gg",0},{"1G",0},{"10G",9},{"10000G",11},{":$",11},{":100",11},
    {"/bbbbb",1},{"/d",1},{"/a",3},{"n",3},{"N",3},
    {"?bbbbb",1},{"?d",1},{"?a",0},{"n",0},{"N",0}})
  {
    if (go.first.back() != 'd')
      CPPUNIT_ASSERT( vi->Command(go.first));
    else
      CPPUNIT_ASSERT(!vi->Command(go.first));
      
    CPPUNIT_ASSERT( stc->GetCurrentLine() == go.second);
  }
}
