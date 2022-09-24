////////////////////////////////////////////////////////////////////////////////
// Name:      test-vi.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/config.h>
#include <wex/core/core.h>
#include <wex/ex/macro-mode.h>
#include <wex/ex/macros.h>
#include <wex/ui/frd.h>
#include <wex/vi/vi.h>

#include "../ex/test.h"

#define ESC "\x1b"

// See stc/test-vi.cpp for testing goto and vim

void change_mode(
  wex::vi*              vi,
  const std::string&    command,
  wex::vi_mode::state_t mode)
{
  REQUIRE(vi->command(command));
  REQUIRE(vi->mode().get() == mode);
}

void change_prep(
  const std::string& command,
  wex::vi*           vi,
  wex::factory::stc* stc)
{
  stc->set_text("xxxxxxxxxx second third\nxxxxxxxx\naaaaaaaaaa\n");
  REQUIRE(stc->get_line_count() == 4);

  REQUIRE(vi->command(command));
  REQUIRE(vi->mode().is_insert());
  REQUIRE(vi->command("zzz"));

  change_mode(vi, ESC, wex::vi_mode::state_t::COMMAND);
  REQUIRE(stc->get_line_count() == 4);
}

TEST_CASE("wex::vi")
{
  auto* stc = get_stc();
  auto* vi  = new wex::vi(get_stc());

  // First load macros.
  REQUIRE(wex::ex::get_macros().load_document());

  SUBCASE("calc")
  {
    stc->set_text("this text contains xx");
    REQUIRE(vi->command("i"));
    REQUIRE(vi->command("=5+5"));
    REQUIRE(vi->command(""));
    REQUIRE(stc->get_text().find("10") != std::string::npos);

    change_mode(vi, ESC, wex::vi_mode::state_t::COMMAND);
    REQUIRE(vi->command("=5+5"));
    REQUIRE(
      wex::ex::get_macros().get_register('0').find("10") != std::string::npos);

    REQUIRE(!vi->command("=5+5+5"));
    REQUIRE(
      wex::ex::get_macros().get_register('0').find("15") == std::string::npos);

    REQUIRE(vi->command("i"));
    REQUIRE(!vi->command(""));
    REQUIRE(vi->command("="));
    REQUIRE(vi->command(""));
  }

  SUBCASE("delete")
  {
    stc->set_text("XXXXX\nYYYYY  \nZZZZZ\n");

    SUBCASE("normal")
    {
      REQUIRE(vi->command("x"));
      REQUIRE(stc->get_text() == "XXXX\nYYYYY  \nZZZZZ\n");
    }

    SUBCASE("block")
    {
      REQUIRE(vi->command("K"));
      REQUIRE(vi->mode().get() == wex::vi_mode::state_t::VISUAL_BLOCK);
      REQUIRE(vi->command("j"));
      REQUIRE(vi->command("j"));
      REQUIRE(vi->command(" "));
      REQUIRE(vi->command("x"));
      REQUIRE(stc->get_text() == "XXXX\nYYYY  \nZZZZ\n");
      change_mode(vi, ESC, wex::vi_mode::state_t::COMMAND);
    }

    SUBCASE("selection")
    {
      REQUIRE(vi->command("/ +"));
      REQUIRE(vi->command("d"));
      REQUIRE(stc->get_text() == "XXXXX\nYYYYY\nZZZZZ\n");
    }
  }

  SUBCASE("change")
  {
    SUBCASE("normal")
    {
      change_prep("cw", vi, stc);
      REQUIRE(stc->GetLineText(0) == "zzzsecond third");

      change_prep("ce", vi, stc);
      REQUIRE(stc->GetLineText(0) == "zzz second third");

      change_prep("2ce", vi, stc);
      REQUIRE(stc->GetLineText(0) == "zzz third");
    }

    SUBCASE("block")
    {
      stc->set_text("xxxxxxxxxx second third\nxxxxxxxxxx\naaaaaaaaaa\n");

      REQUIRE(vi->command("K"));
      REQUIRE(vi->mode().get() == wex::vi_mode::state_t::VISUAL_BLOCK);
      REQUIRE(vi->command("j"));
      REQUIRE(vi->command("j"));
      // Next should be the OK..
      // REQUIRE(vi->command("ce"));
      // REQUIRE(vi->mode().get() == wex::vi_mode::state_t::INSERT_BLOCK);
      // REQUIRE(vi->command("uu"));
      // REQUIRE(stc->get_text() == "uu second third\nuu\nuu\n");
      change_mode(vi, ESC, wex::vi_mode::state_t::COMMAND);
    }

    SUBCASE("block-select")
    {
      stc->set_text("xxxxxxxxxx second third\nxxxxxxxxxx\naaaaaaaaaa\n");

      REQUIRE(vi->command("K"));
      REQUIRE(vi->mode().get() == wex::vi_mode::state_t::VISUAL_BLOCK);
      REQUIRE(vi->command("j"));
      REQUIRE(vi->command("j"));
      REQUIRE(vi->command("l"));
      REQUIRE(vi->command("l"));
      REQUIRE(vi->command("c"));
      REQUIRE(vi->mode().get() == wex::vi_mode::state_t::INSERT_BLOCK);
      vi->mode().command();
      REQUIRE(vi->mode().get() == wex::vi_mode::state_t::COMMAND);
    }
  }

  SUBCASE("find")
  {
    stc->set_text("some text to find another find");
    REQUIRE(vi->mode().is_command());
    REQUIRE(vi->command("/find"));
    REQUIRE(stc->GetCurrentPos() == 17);
    REQUIRE(vi->mode().is_command());
    REQUIRE(vi->command("yb"));
    REQUIRE(vi->mode().is_command());
    REQUIRE(!vi->command("/xfind"));

    stc->DocumentStart();
    REQUIRE(vi->command("2/find"));
    REQUIRE(stc->GetCurrentPos() == 30);
  }

  SUBCASE("insert")
  {
    stc->set_text("aaaaa");
    REQUIRE(vi->mode().is_command());
    REQUIRE(vi->command("i"));
    REQUIRE(vi->mode().is_insert());
    REQUIRE(vi->last_command() == "i");
    REQUIRE(vi->command("xxxxxxxx"));
    REQUIRE(stc->get_text().find("xxxxxxxx") != std::string::npos);
    REQUIRE(vi->last_command() == "i");
    change_mode(vi, ESC, wex::vi_mode::state_t::COMMAND);
    REQUIRE(wex::ex::register_insert() == "xxxxxxxx");
    REQUIRE(vi->last_command() == std::string("ixxxxxxxx") + ESC);
    for (int i = 0; i < 10; i++)
      REQUIRE(vi->command("."));
    REQUIRE(
      stc->get_text().find("xxxxxxxxxxxxxxxxxxxxxxxxxxx") != std::string::npos);

    // insert commands
    std::vector<std::string> commands;
    for (auto& it1 : vi->mode().insert_commands())
    {
      if (it1.first != 'c')
      {
        CAPTURE(it1.first);
        change_mode(
          vi,
          std::string(1, it1.first),
          wex::vi_mode::state_t::INSERT);
        change_mode(vi, ESC, wex::vi_mode::state_t::COMMAND);
        commands.push_back(std::string(1, it1.first));
      }
    }

    // insert commands and delete commands on readonly document
    commands.insert(commands.end(), {"dd", "d0", "d$", "dw", "de"});
    stc->SetReadOnly(true);
    stc->EmptyUndoBuffer();
    stc->SetSavePoint();
    for (auto& it2 : commands)
    {
      REQUIRE(vi->command(it2));
      REQUIRE(vi->mode().is_command());
      REQUIRE(!stc->GetModify());
    }

    // insert command (again)
    stc->SetReadOnly(false);
    change_mode(vi, "i", wex::vi_mode::state_t::INSERT);
    change_mode(vi, ESC, wex::vi_mode::state_t::COMMAND);
    change_mode(vi, "iyyyyy", wex::vi_mode::state_t::INSERT);
    REQUIRE(stc->get_text().find("yyyyy") != std::string::npos);
    REQUIRE(stc->get_text().find("iyyyyy") == std::string::npos);

    change_mode(vi, ESC, wex::vi_mode::state_t::COMMAND);
    const std::string lastcmd = std::string("iyyyyy") + ESC;
    REQUIRE(vi->last_command() == lastcmd);
    REQUIRE(vi->inserted_text() == "yyyyy");
    REQUIRE(vi->command("."));
    REQUIRE(vi->last_command() == lastcmd);
    REQUIRE(vi->last_command() == lastcmd);
    REQUIRE(vi->command("ma"));
    REQUIRE(vi->last_command() == lastcmd);
    REQUIRE(vi->command("100izz"));
    REQUIRE(vi->mode().is_insert());
    REQUIRE(stc->get_text().find("izz") == std::string::npos);

    change_mode(vi, ESC, wex::vi_mode::state_t::COMMAND);
    REQUIRE(stc->get_text().find(std::string(100, 'z')) != std::string::npos);

    // Test insert \n.
    change_mode(vi, "i\n\n\n\n", wex::vi_mode::state_t::INSERT);
    REQUIRE(stc->get_text().find("\n") != std::string::npos);
    REQUIRE(stc->get_text().find("i") == std::string::npos);

    change_mode(vi, ESC, wex::vi_mode::state_t::COMMAND);
    REQUIRE(vi->inserted_text() == "\n\n\n\n");
    REQUIRE(!vi->mode().is_insert());
  }

  SUBCASE("invalid command")
  {
    REQUIRE(!vi->command(":xxx"));
    change_mode(vi, ESC, wex::vi_mode::state_t::COMMAND);
  }

  SUBCASE("is_active")
  {
    REQUIRE(vi->is_active());
    vi->use(wex::ex::OFF);
    REQUIRE(!vi->is_active());
    vi->use(wex::ex::VISUAL);
    REQUIRE(vi->is_active());
  }

  SUBCASE("join")
  {
    stc->set_text("this text contains xx\nand yy");
    REQUIRE(stc->get_line_count() == 2);
    REQUIRE(stc->get_current_line() == 0);

    REQUIRE(vi->command("J"));
    REQUIRE(stc->get_line_count() == 1);
  }

  SUBCASE("macro")
  {
    for (const auto& macro : std::vector<std::vector<std::string>>{
           {"10w"},
           {"dw"},
           {"de"},
           {"yw"},
           {"yk"},
           {"/xx", "rz"}})
    {
      stc->set_text("this text contains xx");

      REQUIRE(vi->command("qt"));
      REQUIRE(wex::ex::get_macros().mode().is_recording());

      std::string all;

      for (auto& command : macro)
      {
        REQUIRE(vi->command(command));
        all += command;
      }

      REQUIRE(vi->command("q"));
      REQUIRE(wex::ex::get_macros().get_register('t') == all);
    }

    REQUIRE(vi->command("@t"));
    REQUIRE(vi->command("@@"));
    REQUIRE(vi->command("."));
    REQUIRE(vi->command("10@t"));
  }

  SUBCASE("maps")
  {
    stc->set_text("this text is not needed");
    vi->command(":map 7 :%d");
    int ctrl_g = 7;
    vi->command(std::string(1, ctrl_g));
    REQUIRE(stc->get_text().empty());
  }

  // Test motion commands: navigate, yank, delete, and change.
  SUBCASE("motion")
  {
    stc->set_text("xxxxxxxxxx second\nxxxxxxxx\naaaaaaaaaa\n");
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

        CAPTURE(motion_command.first);
        CAPTURE(nc);
        REQUIRE(vi->command(nc));

        // test navigate while in block mode
        change_mode(vi, "K", wex::vi_mode::state_t::VISUAL_BLOCK);
        REQUIRE(vi->command(nc));
        REQUIRE(vi->mode().is_visual());
        change_mode(vi, ESC, wex::vi_mode::state_t::COMMAND);
        REQUIRE(vi->mode().is_command());

        // test yank
        std::string mc(
          c == 'f' || c == 't' || c == 'F' || c == 'T' || c == '\'' ? 3 : 2,
          'y');

        mc[0] = 'y';
        mc[1] = c;

        CAPTURE(mc);
        REQUIRE(vi->command(mc));
        REQUIRE(vi->last_command() == mc);
        REQUIRE(vi->mode().is_command());

        // test delete
        mc[0] = 'd';
        CAPTURE(mc);
        REQUIRE(vi->command(mc));
        REQUIRE(vi->last_command() == mc);
        REQUIRE(vi->mode().is_command());

        // test change
        mc[0] = 'c';
        CAPTURE(mc);
        REQUIRE(vi->command(mc));
        REQUIRE(vi->last_command() == mc);
        change_mode(vi, ESC, wex::vi_mode::state_t::COMMAND);
        REQUIRE(vi->mode().is_command());
      }
    }
  }

  SUBCASE("navigate")
  {
    // Test % navigate.
    stc->set_text("{a brace and a close brace}");
    REQUIRE(vi->command("y%"));
    REQUIRE(stc->GetSelectedText().size() == 27);

    // Test delete navigate.
    stc->set_text("xyz");
    REQUIRE(vi->command(std::string(1, WXK_DELETE)));
    REQUIRE(stc->get_text() == "yz");

    // Test record find.
    stc->set_text("abc\nuvw\nxyz\n");

    // Test recording.
    REQUIRE(!wex::ex::get_macros().mode().is_recording());
    vi->command("");
    REQUIRE(vi->command("qa"));
    REQUIRE(wex::ex::get_macros().mode().is_recording());
    REQUIRE(vi->command("/abc"));
    REQUIRE(vi->command("q"));
    REQUIRE(!wex::ex::get_macros().mode().is_recording());
    REQUIRE(vi->command("@a"));
    REQUIRE(vi->command(" "));
    REQUIRE(vi->command("qb"));
    REQUIRE(wex::ex::get_macros().mode().is_recording());
    REQUIRE(vi->command("?abc"));
    REQUIRE(vi->command("q"));
    REQUIRE(!wex::ex::get_macros().mode().is_recording());
    REQUIRE(vi->command("@b"));
    REQUIRE(vi->command(" "));
  }

  // on_char(), on_key_down() : see stc/test-vi.cpp

  SUBCASE("playback")
  {
    REQUIRE(vi->command("qa"));

    REQUIRE(vi->command("atest"));
    change_mode(vi, ESC, wex::vi_mode::state_t::COMMAND);

    REQUIRE(vi->command("q"));

    stc->set_text("");
    REQUIRE(vi->command("@a"));
    REQUIRE(stc->get_text() == "test");
  }

  SUBCASE("put")
  {
    stc->set_text("the chances of anything coming from mars\n");
    REQUIRE(vi->command("yy"));
    REQUIRE(vi->command("p"));
    REQUIRE(vi->command("100p"));
    REQUIRE(
      stc->GetLength() >
      100 * sizeof("the chances of anything coming from mars"));
  }

  SUBCASE("put-special")
  {
    // Put should not put text within a line, but after it, or before it.
    stc->set_text("the chances of anything coming from mars\n");
    vi->command("$");
    vi->command("h");
    vi->command("h");
    vi->command("yy");
    REQUIRE(
      wex::ex::get_macros().get_register('0').find("the chances of anything") !=
      std::string::npos);

    vi->command("p");
    vi->command("P");
    REQUIRE(
      stc->get_text().find("the chances of anything coming from mars") !=
      std::string::npos);
    REQUIRE(stc->get_text().find("mathe") == std::string::npos);
  }

  SUBCASE("registers")
  {
    stc->set_text("");
    const std::string ctrl_r = "\x12";
    REQUIRE(vi->command("i"));
    REQUIRE(vi->command(ctrl_r + "_"));
    change_mode(vi, ESC, wex::vi_mode::state_t::COMMAND);

    stc->set_text("XXXXX");
    REQUIRE(vi->command("dd"));
    REQUIRE(vi->command("i"));
    REQUIRE(vi->command(ctrl_r + "1"));
    change_mode(vi, ESC, wex::vi_mode::state_t::COMMAND);
    REQUIRE(stc->get_text() == "XXXXX");

    stc->set_text("YYYYY");
    REQUIRE(vi->command("dd"));
    REQUIRE(vi->command("i"));
    REQUIRE(vi->command(ctrl_r + "2"));
    REQUIRE(vi->command("2"));
    change_mode(vi, ESC, wex::vi_mode::state_t::COMMAND);
    REQUIRE(stc->get_text().find("XXXXX") != std::string::npos);
  }

  SUBCASE("replace")
  {
    stc->set_text("XXXXX\nYYYYY\nZZZZZ\n");

    SUBCASE("normal")
    {
      REQUIRE(vi->command("3rx"));
      REQUIRE(stc->get_text() == "xxxXX\nYYYYY\nZZZZZ\n");
    }

    SUBCASE("block")
    {
      REQUIRE(vi->command("K"));
      REQUIRE(vi->command("j"));
      REQUIRE(vi->command("j"));
      REQUIRE(vi->command("3rx"));
      REQUIRE(stc->get_text() == "xxxXX\nxxxYY\nxxxZZ\n");
      change_mode(vi, ESC, wex::vi_mode::state_t::COMMAND);
    }
  }

  SUBCASE("substitute")
  {
    stc->set_text("999");
    REQUIRE(vi->command(":1,$s/xx/yy/g")); // so &, ~ are ok
    REQUIRE(vi->command("i"));
    REQUIRE(vi->mode().is_insert());
    REQUIRE(vi->command("b"));
    change_mode(vi, ESC, wex::vi_mode::state_t::COMMAND);
    REQUIRE(stc->get_text().find("b") != std::string::npos);
    stc->set_text("xxxxxxxxxx\nxxxxxxxx\naaaaaaaaaa\n");
    REQUIRE(vi->command(":.="));
    REQUIRE(vi->command(":1,$s/xx/yy/g"));
    REQUIRE(stc->get_text().find("xx") == std::string::npos);
    REQUIRE(stc->get_text().find("yy") != std::string::npos);
  }

  SUBCASE("tags")
  {
    stc->set_text("no tag");
    REQUIRE(vi->command("Q"));
    stc->set_text("wex::test_app");
    REQUIRE(vi->command("Q"));
    REQUIRE(vi->command("\x17"));
    vi->append_insert_command("xyz");
    vi->append_insert_text("hello world");
  }

  SUBCASE("variable")
  {
    stc->set_text("");
    REQUIRE(vi->command("@Date@"));
    REQUIRE(stc->get_text().find("Date") == std::string::npos);

    stc->set_text("");
    REQUIRE(vi->command("@Year@"));
    REQUIRE(stc->get_text().find("20") != std::string::npos);
    REQUIRE(stc->get_lexer().set("cpp"));

    stc->set_text("");
    REQUIRE(vi->command("@Cb@"));
    REQUIRE(vi->command("@Ce@"));
    REQUIRE(stc->get_text().find("//") != std::string::npos);

    stc->set_text("");
    REQUIRE(vi->command("@Cl@"));
    REQUIRE(stc->get_text().find("//") != std::string::npos);
    REQUIRE(vi->command("@Nl@"));
  }

  SUBCASE("visual mode")
  {
    stc->set_text("123456789");
    vi->command("v");
    REQUIRE(vi->mode().is_visual());

    vi->visual_extend(0, 10);
    REQUIRE(vi->get_stc()->get_selected_text() == "123456789");
    vi->mode().escape();

    vi->command("gg");
    vi->command("v");
    REQUIRE(vi->mode().is_visual());
    vi->command("llll");
    REQUIRE(vi->command("y"));
    REQUIRE(vi->mode().is_command());
    REQUIRE(vi->register_text() == "1234");
  }

  SUBCASE("yank")
  {
    stc->set_text("xxxxxxxxxx second\nxxxxxxxx\naaaaaaaaaa\n");
    REQUIRE(vi->command("yw"));
    REQUIRE(vi->get_stc()->get_selected_text() == "xxxxxxxxxx ");
    REQUIRE(vi->command("w"));
    REQUIRE(vi->command("x"));
    REQUIRE(stc->get_text().find("second") != std::string::npos);
    REQUIRE(vi->command("p"));
    REQUIRE(stc->get_text().find("second") != std::string::npos);
  }

  SUBCASE("others")
  {
    // Test abbreviate.
    for (auto& abbrev : wex::test::get_abbreviations())
    {
      REQUIRE(vi->command(":ab " + abbrev.first + " " + abbrev.second));
      REQUIRE(vi->command("iabbreviation " + abbrev.first + " "));
      change_mode(vi, ESC, wex::vi_mode::state_t::COMMAND);
      REQUIRE(vi->inserted_text() == "abbreviation " + abbrev.first + " ");
      REQUIRE(
        stc->get_text().find("abbreviation " + abbrev.second) !=
        std::string::npos);
      REQUIRE(stc->get_text().find(abbrev.first) == std::string::npos);
    }

    // Test special delete commands.
    for (auto& delete_command : std::vector<std::string>{"dgg", "3dw", "d3w"})
    {
      REQUIRE(vi->command(delete_command));
      REQUIRE(vi->last_command() == delete_command);
    }

    vi->reset_search_flags();
    wex::config(_("stc.Wrap scan")).set(true);

    // Test fold.
    for (auto& fold : std::vector<std::string>{"zo", "zc", "zE", "zf"})
    {
      REQUIRE(vi->command(fold));
      REQUIRE(vi->last_command() != fold);
    }

    // Test other commands (ZZ not tested).
    for (auto& other_command : vi->other_commands())
    {
      if (other_command.first == "ZZ")
        continue;

      if (vi->mode().is_insert())
      {
        vi->mode().escape();
      }

      stc->set_text("first second\nthird\nfourth\n");

      if (
        !isalpha(other_command.first.front()) &&
        other_command.first.front() != '\x12' &&
        other_command.first.front() != '@')
      {
        for (auto c : other_command.first)
        {
          // prevent wex::browser_search
          if (c == 'U')
          {
            continue;
          }

          if (c != '\t')
            REQUIRE(vi->command(std::string(1, c)));
          else
            REQUIRE(!vi->command(std::string(1, c)));
        }
      }
      else
      {
        const std::string oc(
          other_command.first == "m" || other_command.first == "q" ||
              other_command.first == "r" || other_command.first == "\x12" ||
              other_command.first == "@" ?
            other_command.first + "a" :
            other_command.first);
        CAPTURE(oc);

        if (oc != "z")
          REQUIRE(vi->command(oc));
        else if (oc != "qa")
        {
          REQUIRE(!vi->command(oc));
        }
      }
    }

    if (wex::ex::get_macros().mode().is_recording())
    {
      vi->command("q");
    }
  }
}
