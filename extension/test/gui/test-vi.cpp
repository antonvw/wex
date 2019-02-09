////////////////////////////////////////////////////////////////////////////////
// Name:      test-vi.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/vi.h>
#include <wex/config.h>
#include <wex/frd.h>
#include <wex/managedframe.h>
#include <wex/vi-macros.h>
#include <wex/vi-macros-mode.h>
#include <wex/stc.h>
#include "test.h"

#define ESC "\x1b"

void ChangeMode(wex::vi* vi, const std::string& command, wex::vi_mode::state_t mode)
{
  REQUIRE( vi->command(command));
  REQUIRE( vi->mode().get() == mode);
}

TEST_CASE("wex::vi")
{
  wex::stc* stc = new wex::stc(
    std::string("// vi: set ts=120 "
                "// this is a modeline"));
  add_pane(frame(), stc);
  wex::vi* vi = &stc->get_vi();
  wxKeyEvent event(wxEVT_CHAR);
  
  // Test for modeline support.
  REQUIRE(stc->GetTabWidth() == 120);
  REQUIRE( vi->is_active());
  REQUIRE( vi->mode().normal());

  // Test WXK_NONE.
  event.m_uniChar = WXK_NONE;
  REQUIRE( vi->on_char(event));
  
  // First i enters insert mode, so is handled by vi, not to be skipped.
  event.m_uniChar = 'i';
  REQUIRE(!vi->on_char(event));
  REQUIRE( vi->mode().insert());
  REQUIRE( vi->mode().string() == "insert");
  // Second i (and more) all handled by vi.
  for (int i = 0; i < 10; i++) REQUIRE(!vi->on_char(event));

  // Test control keys.
  for (const auto& control_key : std::vector<int> {
    WXK_CONTROL_B,WXK_CONTROL_E,WXK_CONTROL_F,WXK_CONTROL_G,
    WXK_CONTROL_J,WXK_CONTROL_P,WXK_CONTROL_Q})
  {
    event.m_uniChar = control_key;
    REQUIRE( vi->on_key_down(event));
    REQUIRE(!vi->on_char(event));
  }
  
  // Test change number.
  event.m_uniChar = WXK_CONTROL_E;
  for (const auto& number: std::vector<std::string> {
    "101", "0xf7", "077", "-99"})
  {
    stc->set_text("number: " + number);
    vi->command("gg");
    vi->command("2w");
    REQUIRE( vi->on_key_down(event));
    REQUIRE(!vi->on_char(event));
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
//    REQUIRE(!vi->on_key_down(event));
    ChangeMode( vi, ESC, wex::vi_mode::state_t::NORMAL);
  }
  event.m_keyCode = WXK_NONE;
  REQUIRE( vi->on_key_down(event));

  // Test navigate with [ and ].
  event.m_uniChar = '[';
  REQUIRE(!vi->on_char(event));
  event.m_uniChar= ']';
  REQUIRE(!vi->on_char(event));
  vi->get_stc()->AppendText("{}");
  event.m_uniChar = '[';
  REQUIRE(!vi->on_char(event));
  event.m_uniChar= ']';
  REQUIRE(!vi->on_char(event));
  
  // Test insert command.
  stc->set_text("aaaaa");
  REQUIRE( vi->mode().normal());
  REQUIRE( vi->command("i"));
  REQUIRE( vi->mode().insert());
  REQUIRE( vi->last_command() == "i");
  REQUIRE( vi->command("xxxxxxxx"));
  REQUIRE( stc->GetText().Contains("xxxxxxxx"));
  REQUIRE( vi->last_command() == "i");
  ChangeMode( vi, ESC, wex::vi_mode::state_t::NORMAL);
  REQUIRE( vi->register_insert() == "xxxxxxxx");
  REQUIRE( vi->last_command() == std::string("ixxxxxxxx") + ESC);
  for (int i = 0; i < 10; i++)
    REQUIRE( vi->command("."));
  REQUIRE( stc->GetText().Contains("xxxxxxxxxxxxxxxxxxxxxxxxxxx"));
  
  // Test insert commands.
  std::vector<std::string> commands;
  for (auto& it1 : vi->mode().insert_commands())
  {
    if (it1.first != 'c')
    {
      CAPTURE( it1.first);
      ChangeMode( vi, std::string(1, it1.first), wex::vi_mode::state_t::INSERT);
      ChangeMode( vi, ESC, wex::vi_mode::state_t::NORMAL);
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
    REQUIRE( vi->command(it2));
    REQUIRE( vi->mode().normal());
    REQUIRE(!stc->GetModify());
  }

  // Test insert on hexmode document.
  stc->SetReadOnly(false);
  stc->get_hexmode().set(true);
  REQUIRE( stc->is_hexmode());
  REQUIRE(!stc->GetModify());
  REQUIRE( vi->command("a") );
  REQUIRE( vi->mode().insert());
  REQUIRE(!vi->command("xxxxxxxx"));
  ChangeMode( vi, ESC, wex::vi_mode::state_t::NORMAL);
  REQUIRE(!stc->GetModify());
  stc->get_hexmode().set(false);
  REQUIRE(!stc->is_hexmode());
  REQUIRE(!stc->GetModify());
  stc->SetReadOnly(false);

  // Test insert command.  
  ChangeMode( vi, "i", wex::vi_mode::state_t::INSERT);
  ChangeMode( vi, ESC, wex::vi_mode::state_t::NORMAL);
  ChangeMode( vi, "iyyyyy", wex::vi_mode::state_t::INSERT);
  REQUIRE( stc->GetText().Contains("yyyyy"));
  REQUIRE(!stc->GetText().Contains("iyyyyy"));
  ChangeMode( vi, ESC, wex::vi_mode::state_t::NORMAL);
  const std::string lastcmd = std::string("iyyyyy") + ESC;
  REQUIRE( vi->last_command() == lastcmd);
  REQUIRE( vi->inserted_text() == "yyyyy");
  REQUIRE( vi->command("."));
  REQUIRE( vi->last_command() == lastcmd);
  REQUIRE(!vi->command(";"));
  REQUIRE( vi->last_command() == lastcmd);
  REQUIRE( vi->command("ma"));
  REQUIRE( vi->last_command() == lastcmd);
  REQUIRE( vi->command("100izz"));
  REQUIRE( vi->mode().insert());
  REQUIRE(!stc->GetText().Contains("izz"));
  ChangeMode( vi, ESC, wex::vi_mode::state_t::NORMAL);
  REQUIRE( stc->GetText().Contains(std::string(100, 'z')));

  // Test insert \n.
  ChangeMode( vi, "i\n\n\n\n", wex::vi_mode::state_t::INSERT);
  REQUIRE( stc->GetText().Contains("\n"));
  REQUIRE(!stc->GetText().Contains("i"));
  ChangeMode( vi, ESC, wex::vi_mode::state_t::NORMAL);
  REQUIRE( vi->inserted_text() == "\n\n\n\n");
  
  stc->set_text("");
  event.m_uniChar = 'i';
  REQUIRE(!vi->on_char(event));
  REQUIRE( vi->mode().insert());
  REQUIRE( vi->inserted_text().empty());
  REQUIRE( vi->mode().insert());
  event.m_uniChar = WXK_RETURN;
  REQUIRE( vi->on_key_down(event));
  REQUIRE(!vi->on_char(event));
  ChangeMode( vi, ESC, wex::vi_mode::state_t::NORMAL);
#ifndef __WXOSX__  
  REQUIRE( wxString(vi->inserted_text()).Contains("\n"));
#endif
  
  // Test abbreviate.
  for (auto& abbrev : get_abbreviations())
  {
    REQUIRE( vi->command(":ab " + abbrev.first + " " + abbrev.second));
    REQUIRE( vi->command("iabbreviation " + abbrev.first + " "));
    ChangeMode( vi, ESC, wex::vi_mode::state_t::NORMAL);
    REQUIRE( vi->inserted_text() == "abbreviation "  + abbrev.first + " ");
    REQUIRE( stc->GetText().Contains("abbreviation " + abbrev.second));
    REQUIRE(!stc->GetText().Contains(abbrev.first));
  }
 
  // Test maps.
  stc->set_text("this text is not needed");
  vi->command(":map 7 :%d");
  int ctrl_g = 7;
  vi->command(std::string(1, ctrl_g));
  REQUIRE(stc->GetText().empty());

  // Test substitute command.
  stc->set_text("999");
  REQUIRE( vi->command(":1,$s/xx/yy/g")); // so &, ~ are ok
  REQUIRE( vi->command("i"));
  REQUIRE( vi->mode().insert());
  REQUIRE( vi->command("b"));
  ChangeMode( vi, ESC, wex::vi_mode::state_t::NORMAL);
  REQUIRE( stc->GetText().Contains("b"));
  stc->set_text("xxxxxxxxxx\nxxxxxxxx\naaaaaaaaaa\n");
  REQUIRE( vi->command(":.="));
  REQUIRE( vi->command(":1,$s/xx/yy/g"));
  REQUIRE(!stc->GetText().Contains("xx"));
  REQUIRE( stc->GetText().Contains("yy"));
  
  // Test change command.
  stc->set_text("xxxxxxxxxx second\nxxxxxxxx\naaaaaaaaaa\n");
  REQUIRE( stc->GetLineCount() == 4);
  REQUIRE( vi->command(":1"));
  REQUIRE( vi->command("cw"));
  REQUIRE( vi->mode().insert());
  REQUIRE( vi->command("zzz"));
  ChangeMode( vi, ESC, wex::vi_mode::state_t::NORMAL);
  REQUIRE( stc->GetLineCount() == 4);
  REQUIRE( stc->GetLineText(0) == "zzzsecond");
  stc->set_text("xxxxxxxxxx second\nxxxxxxxx\naaaaaaaaaa\n");
  REQUIRE( vi->command(":1"));
  REQUIRE( vi->command("ce"));
  REQUIRE( vi->mode().insert());
  REQUIRE( vi->command("zzz"));
  ChangeMode( vi, ESC, wex::vi_mode::state_t::NORMAL);
  REQUIRE( stc->GetLineCount() == 4);
  REQUIRE( stc->GetLineText(0) == "zzz second");
  stc->set_text("xxxxxxxxxx second third\nxxxxxxxx\naaaaaaaaaa\n");
  REQUIRE( vi->command(":1"));
  REQUIRE( vi->command("2ce"));
  REQUIRE( vi->mode().insert());
  REQUIRE( vi->command("zzz"));
  ChangeMode( vi, ESC, wex::vi_mode::state_t::NORMAL);
  REQUIRE( stc->GetLineCount() == 4);
  REQUIRE( stc->GetLineText(0) == "zzz third");

  // Test special delete commands.
  for (auto& delete_command : std::vector<std::string> {"dgg","3dw", "d3w"})
  {
    REQUIRE( vi->command(delete_command));
    REQUIRE( vi->last_command() == delete_command);
  }

  // Test yank commands.
  stc->set_text("xxxxxxxxxx second\nxxxxxxxx\naaaaaaaaaa\n");
  REQUIRE( vi->command(":1"));
  REQUIRE( vi->command("yw"));
  REQUIRE( vi->get_selected_text() == "xxxxxxxxxx ");
  REQUIRE( vi->command("w"));
  REQUIRE( vi->command("x"));
  REQUIRE( stc->GetText().Contains("second"));
  REQUIRE( vi->command("p"));
  REQUIRE( stc->GetText().Contains("second"));
  
  vi->reset_search_flags();
  wex::config(_("Wrap scan")).set(true);

  // Test motion commands: navigate, yank, delete, and change.
  wex::find_replace_data::get()->set_find_string("xx");

  for (auto& motion_command : vi->motion_commands())
  {
    for (auto c : motion_command.first)
    {
      stc->set_text("xxxxxxxxxx\nyyyyyyyyyy\n"
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
      REQUIRE( vi->command(nc));
      
      // test navigate while in rect mode
      ChangeMode( vi, "K", wex::vi_mode::state_t::VISUAL_RECT);
      REQUIRE( vi->command( nc ));
      REQUIRE( vi->mode().visual());
      ChangeMode( vi, ESC, wex::vi_mode::state_t::NORMAL);
      REQUIRE( vi->mode().normal());
      
      // test yank
      std::string mc(
        c == 'f' || c == 't' ||
        c == 'F' || c == 'T' ||
        c == '\'' ? 3: 2, 'y');

      mc[0] = 'y';
      mc[1] = c;

      CAPTURE( mc);
      REQUIRE( vi->command(mc));
      REQUIRE( vi->last_command() == mc);
      REQUIRE( vi->mode().normal());

      // test delete
      mc[0] = 'd';
      CAPTURE( mc);
      REQUIRE( vi->command(mc));
      REQUIRE( vi->last_command() == mc);
      REQUIRE( vi->mode().normal());
      
      // test change
      mc[0] = 'c';
      CAPTURE( mc);
      REQUIRE( vi->command(mc));
      REQUIRE( vi->last_command() == mc);
      ChangeMode( vi, ESC, wex::vi_mode::state_t::NORMAL);
      REQUIRE( vi->mode().normal());
    }
  }

  // Test find.
  stc->set_text("some text to find");
  REQUIRE( vi->mode().normal());
  REQUIRE( vi->command("/find"));
  REQUIRE( vi->mode().normal());
  REQUIRE( vi->command("yb"));
  REQUIRE( vi->mode().normal());
  REQUIRE(!vi->command("/xfind"));

  // Test % navigate.
  stc->set_text("{a brace and a close brace}");
  REQUIRE( vi->command("y%"));
  REQUIRE( stc->GetSelectedText().size() == 27);
  
  // Test delete navigate.
  stc->set_text("xyz");
  REQUIRE( vi->command(std::string(1, WXK_DELETE)));
  REQUIRE( stc->GetText() == "yz");

  // Test record find.
  stc->set_text("abc\nuvw\nxyz\n");

  // Test recording.
  REQUIRE(!vi->get_macros().mode()->is_recording());
  vi->command("");
  REQUIRE( vi->command("qa"));
  REQUIRE( vi->get_macros().mode()->is_recording());
  REQUIRE( vi->command("/abc"));
  REQUIRE( vi->command("q"));
  REQUIRE(!vi->get_macros().mode()->is_recording());
  REQUIRE( vi->command("@a"));
  REQUIRE( vi->command(" "));
  REQUIRE( vi->command("qb"));
  REQUIRE( vi->get_macros().mode()->is_recording());
  REQUIRE( vi->command("?abc"));
  REQUIRE( vi->command("q"));
  REQUIRE(!vi->get_macros().mode()->is_recording());
  REQUIRE( vi->command("@b"));
  REQUIRE( vi->command(" "));

  // Test fold.
  for (auto& fold: std::vector<std::string> {"zo", "zc", "zE", "zf"})
  {
    REQUIRE( vi->command(fold));
    REQUIRE( vi->last_command() != fold);
  }
  
  // Test other commands (ZZ not tested).
  for (auto& other_command : vi->other_commands())
  {
    stc->set_text("first second\nthird\nfourth\n");

    if (!isalpha(other_command.first.front()) && 
        other_command.first.front() != '\x12' &&
        other_command.first.front() != '@')
    {
      for (auto c : other_command.first)
      {
        // prevent wex::browser_search (for travis)
        if (c == 'U')
        {
          continue;
        }

        if (c != '\t')
          REQUIRE( vi->command(std::string(1, c)));
        else
          REQUIRE(!vi->command(std::string(1, c)));
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
        REQUIRE( vi->command(oc));
      else if (oc != "qa")
        REQUIRE(!vi->command(oc));
    }
  }

  if (vi->get_macros().mode()->is_recording())
  {
    vi->command("q");
  }

  // Special put test. 
  // Put should not put text within a line, but after it, or before it.
  stc->set_text("the chances of anything coming from mars\n");
  vi->command("$");
  vi->command("h");
  vi->command("h");
  vi->command("yy");
  REQUIRE( 
    wxString(stc->get_vi().get_macros().get_register('0')).Contains("the chances of anything"));
  vi->command("p");
  vi->command("P");
  REQUIRE( stc->GetText().Contains(
    "the chances of anything coming from mars"));
  REQUIRE(!stc->GetText().Contains("mathe"));

  // Test calc.
  stc->set_text("this text contains xx");
  vi->command("i");
  vi->command("=5+5");
  vi->command("");
  REQUIRE( stc->GetText().Contains("10"));
  ChangeMode( vi, ESC, wex::vi_mode::state_t::NORMAL);
  vi->command("=5+5");
  REQUIRE( wxString(stc->get_vi().get_macros().get_register('0')).Contains("10"));
  vi->command("=5+5+5");
  REQUIRE( wxString(stc->get_vi().get_macros().get_register('0')).Contains("15"));

  // Test macro.
  // First load macros.
  REQUIRE( wex::vi_macros::load_document());
  for (const auto& macro : std::vector< std::vector< std::string> > {
    {"10w"},
    {"dw"},
    {"de"},
    {"yw"},
    {"yk"},
    {"/xx","rz"}})  
  {
    stc->set_text("this text contains xx");
    
    REQUIRE( vi->command("qt"));
    REQUIRE( vi->get_macros().mode()->is_recording());
    
    std::string all;
    
    for (auto& command : macro)
    {
      REQUIRE( vi->command(command));
      all += command;
    }

    REQUIRE( vi->command("q"));
    REQUIRE( stc->get_vi().get_macros().get_register('t') == all);
  }

  REQUIRE( vi->command("@t"));
  REQUIRE( vi->command("@@"));
  REQUIRE( vi->command("."));
  REQUIRE( vi->command("10@t"));

  // Next should be OK, but crashes due to input expand variable.
  //REQUIRE( vi->command("@hdr@"));
  
  // Test variables.
  stc->set_text("");
  REQUIRE( vi->command("@Date@"));
  REQUIRE(!stc->GetText().Contains("Date"));
  stc->set_text("");
  REQUIRE( vi->command("@Year@"));
  REQUIRE( stc->GetText().Contains("20"));
//  REQUIRE(!vi->command("@xxx@"));
  REQUIRE( stc->get_lexer().set("cpp"));
  stc->set_text("");
  REQUIRE( vi->command("@Cb@"));
  REQUIRE( vi->command("@Ce@"));
  REQUIRE( stc->GetText().Contains("//"));
  stc->set_text("");
  REQUIRE( vi->command("@Cl@"));
  REQUIRE( stc->GetText().Contains("//"));
  REQUIRE( vi->command("@Nl@"));
  
  // Test illegal command.
  REQUIRE(!vi->command(":xxx"));
  ChangeMode( vi, ESC, wex::vi_mode::state_t::NORMAL);

  // Test registers
  stc->get_file().file_new("test.h");
  stc->set_text("");
  const std::string ctrl_r = "\x12";
  REQUIRE( vi->command("i"));
  REQUIRE( vi->command(ctrl_r + "_"));
  ChangeMode( vi, ESC, wex::vi_mode::state_t::NORMAL);
  
  stc->set_text("");
  REQUIRE( vi->command("i"));
  REQUIRE( vi->command(ctrl_r + "%"));
  ChangeMode( vi, ESC, wex::vi_mode::state_t::NORMAL);
  REQUIRE( stc->GetText() == "test.h");
  
  REQUIRE( vi->command("yy"));
  stc->set_text("");
  REQUIRE( vi->command("i"));
  REQUIRE( vi->command(ctrl_r + "0"));
  ChangeMode( vi, ESC, wex::vi_mode::state_t::NORMAL);
  REQUIRE( stc->GetText() == "test.h");
  
  stc->set_text("XXXXX");
  REQUIRE( vi->command("dd"));
  REQUIRE( vi->command("i"));
  REQUIRE( vi->command(ctrl_r + "1"));
  ChangeMode( vi, ESC, wex::vi_mode::state_t::NORMAL);
  REQUIRE( stc->GetText() == "XXXXX");
  
  stc->set_text("YYYYY");
  REQUIRE( vi->command("dd"));
  REQUIRE( vi->command("i"));
  REQUIRE( vi->command(ctrl_r + "2"));
  REQUIRE( vi->command("2"));
  ChangeMode( vi, ESC, wex::vi_mode::state_t::NORMAL);
  REQUIRE( stc->GetText().Contains("XXXXX"));
  
  // Test visual modes.
  for (const auto& visual : std::vector<std::pair<std::string, wex::vi_mode::state_t>> {
    {"v",wex::vi_mode::state_t::VISUAL},
    {"V",wex::vi_mode::state_t::VISUAL_LINE},
    {"K",wex::vi_mode::state_t::VISUAL_RECT}})
  {
    ChangeMode( vi, visual.first, visual.second);
    ChangeMode( vi, "jjj", visual.second);
    ChangeMode( vi, visual.first, visual.second); // second has no effect
    // enter illegal command
    vi->command("g");
    vi->command("j");
    ChangeMode( vi, ESC, wex::vi_mode::state_t::NORMAL);
    
    event.m_uniChar = visual.first[0];
    REQUIRE(!vi->on_char(event));
    REQUIRE( vi->mode().get() == visual.second);
    ChangeMode( vi, ESC, wex::vi_mode::state_t::NORMAL);
  }
  
  stc->set_text("123456789");
  vi->command("v");
  REQUIRE( vi->mode().visual());
  vi->visual_extend(0, 10);
  REQUIRE( vi->get_selected_text() == "123456789");
  vi->mode().escape();

  // Test goto, /, ?, n and N.
  stc->set_text("aaaaa\nbbbbb\nccccc\naaaaa\ne\nf\ng\nh\ni\nj\nk\n");
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
      REQUIRE( vi->command(go.first));
    else
      REQUIRE(!vi->command(go.first));
    
    if (go.first[0] == '/' || go.first[0] == '?')
    {
      // A / or ? should not set a last command.
      REQUIRE( vi->last_command()[0] != go.first[0]);
    }

    REQUIRE( stc->GetCurrentLine() == go.second);
  }
  
  // test ctags
  stc->set_text("no tag");
  REQUIRE( vi->command("Q"));
  stc->set_text("wex::test_app");
  REQUIRE( vi->command("Q"));
  REQUIRE( vi->command("S"));

  vi->append_insert_command("xyz");
}
