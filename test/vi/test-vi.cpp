////////////////////////////////////////////////////////////////////////////////
// Name:      test-vi.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/core.h>
#include <wex/ex/macro-mode.h>
#include <wex/ex/macros.h>
#include <wex/ex/util.h>

#include "../ex/test.h"
#include "test.h"

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

  change_mode(vi, wex::esc(), wex::vi_mode::state_t::COMMAND);
  REQUIRE(stc->get_line_count() == 4);
}

// currently contains mode change and control char commands

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
    REQUIRE(stc->get_text().contains("10"));

    change_mode(vi, wex::esc(), wex::vi_mode::state_t::COMMAND);
    REQUIRE(vi->command("=5+5"));
    REQUIRE(wex::ex::get_macros().get_register('0').contains("10"));

    REQUIRE(!vi->command("=5+5+5"));
    REQUIRE(!wex::ex::get_macros().get_register('0').contains("15"));

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
      change_mode(vi, wex::esc(), wex::vi_mode::state_t::COMMAND);
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

    stc->set_text("xxxxxxxxxx second third\nxxxxxxxxxx\naaaaaaaaaa\n");

    REQUIRE(vi->command("K"));
    REQUIRE(vi->mode().get() == wex::vi_mode::state_t::VISUAL_BLOCK);
    REQUIRE(vi->command("j"));
    REQUIRE(vi->command("j"));

    SUBCASE("block")
    {
      // Next should be the OK..
      // REQUIRE(vi->command("ce"));
      // REQUIRE(vi->mode().get() == wex::vi_mode::state_t::INSERT_BLOCK);
      // REQUIRE(vi->command("uu"));
      // REQUIRE(stc->get_text() == "uu second third\nuu\nuu\n");
      change_mode(vi, wex::esc(), wex::vi_mode::state_t::COMMAND);
    }

    SUBCASE("block-select")
    {
      REQUIRE(vi->command("l"));
      REQUIRE(vi->command("l"));
      REQUIRE(vi->command("c"));
      REQUIRE(vi->mode().get() == wex::vi_mode::state_t::INSERT_BLOCK);
      vi->mode().command();
      REQUIRE(vi->mode().get() == wex::vi_mode::state_t::COMMAND);
    }
  }

  SUBCASE("insert")
  {
    stc->set_text("aaaaa");
    REQUIRE(vi->mode().is_command());
    REQUIRE(vi->command("i"));
    REQUIRE(vi->mode().is_insert());
    REQUIRE(vi->last_command() == "i");
    REQUIRE(vi->command("xxxxxxxx"));
    REQUIRE(stc->get_text().contains("xxxxxxxx"));
    REQUIRE(vi->last_command() == "i");
    change_mode(vi, wex::esc(), wex::vi_mode::state_t::COMMAND);
    REQUIRE(wex::ex::register_insert() == "xxxxxxxx");
    REQUIRE(vi->last_command() == "ixxxxxxxx" + wex::esc());
    for (int i = 0; i < 10; i++)
      REQUIRE(vi->command("."));
    REQUIRE(stc->get_text().contains("xxxxxxxxxxxxxxxxxxxxxxxxxxx"));

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
        change_mode(vi, wex::esc(), wex::vi_mode::state_t::COMMAND);
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
    change_mode(vi, wex::esc(), wex::vi_mode::state_t::COMMAND);
    change_mode(vi, "iyyyyy", wex::vi_mode::state_t::INSERT);
    REQUIRE(stc->get_text().contains("yyyyy"));
    REQUIRE(!stc->get_text().contains("iyyyyy"));

    change_mode(vi, wex::esc(), wex::vi_mode::state_t::COMMAND);
    const auto lastcmd = "iyyyyy" + wex::esc();
    REQUIRE(vi->last_command() == lastcmd);
    REQUIRE(vi->inserted_text() == "yyyyy");
    REQUIRE(vi->command("."));
    REQUIRE(vi->last_command() == lastcmd);
    REQUIRE(vi->last_command() == lastcmd);
    REQUIRE(vi->command("ma"));
    REQUIRE(vi->last_command() == lastcmd);
    REQUIRE(vi->command("100izz"));
    REQUIRE(vi->mode().is_insert());
    REQUIRE(!stc->get_text().contains("izz"));

    change_mode(vi, wex::esc(), wex::vi_mode::state_t::COMMAND);
    REQUIRE(stc->get_text().contains(std::string(100, 'z')));

    // Test insert \n.
    change_mode(vi, "i\n\n\n\n", wex::vi_mode::state_t::INSERT);
    REQUIRE(stc->get_text().contains("\n"));
    REQUIRE(!stc->get_text().contains("i"));

    change_mode(vi, wex::esc(), wex::vi_mode::state_t::COMMAND);
    REQUIRE(vi->inserted_text() == "\n\n\n\n");
    REQUIRE(!vi->mode().is_insert());
  }

  SUBCASE("invalid command")
  {
    REQUIRE(!vi->command(":xxx"));
    change_mode(vi, wex::esc(), wex::vi_mode::state_t::COMMAND);
  }

  SUBCASE("is_active")
  {
    REQUIRE(vi->is_active());
    vi->use(wex::ex::OFF);
    REQUIRE(!vi->is_active());
    vi->use(wex::ex::VISUAL);
    REQUIRE(vi->is_active());
  }

  SUBCASE("maps")
  {
    stc->set_text("this text is not needed");
    vi->command(":map " + std::to_string(WXK_CONTROL_G) + " :%d");
    vi->command(wex::k_s(WXK_CONTROL_G));
    REQUIRE(stc->get_text().empty());
  }

  // on_char(), on_key_down() : see stc/test-vi.cpp

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
    REQUIRE(wex::ex::get_macros().get_register('0').contains(
      "the chances of anything"));

    vi->command("p");
    vi->command("P");
    REQUIRE(
      stc->get_text().contains("the chances of anything coming from mars"));
    REQUIRE(!stc->get_text().contains("mathe"));
  }

  SUBCASE("registers")
  {
    stc->set_text("");
    const std::string ctrl_r = "\x12";
    REQUIRE(vi->command("i"));
    REQUIRE(vi->command(ctrl_r + "_"));
    change_mode(vi, wex::esc(), wex::vi_mode::state_t::COMMAND);

    stc->set_text("XXXXX");
    REQUIRE(vi->command("dd"));
    REQUIRE(vi->command("i"));
    REQUIRE(vi->command(ctrl_r + "1"));
    change_mode(vi, wex::esc(), wex::vi_mode::state_t::COMMAND);
    REQUIRE(stc->get_text() == "XXXXX");

    stc->set_text("YYYYY");
    REQUIRE(vi->command("dd"));
    REQUIRE(vi->command("i"));
    REQUIRE(vi->command(ctrl_r + "2"));
    REQUIRE(vi->command("2"));
    change_mode(vi, wex::esc(), wex::vi_mode::state_t::COMMAND);
    REQUIRE(stc->get_text().contains("XXXXX"));
  }

  SUBCASE("substitute")
  {
    stc->set_text("999");
    REQUIRE(vi->command(":1,$s/xx/yy/g")); // so &, ~ are ok
    REQUIRE(vi->command("i"));
    REQUIRE(vi->mode().is_insert());
    REQUIRE(vi->command("b"));
    change_mode(vi, wex::esc(), wex::vi_mode::state_t::COMMAND);
    REQUIRE(stc->get_text().contains("b"));
    stc->set_text("xxxxxxxxxx\nxxxxxxxx\naaaaaaaaaa\n");
    REQUIRE(vi->command(":.="));
    REQUIRE(vi->command(":1,$s/xx/yy/g"));
    REQUIRE(!stc->get_text().contains("xx"));
    REQUIRE(stc->get_text().contains("yy"));
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

    // visual mode and brace match
    stc->set_text("some text\n{and text within brace} some text");
    vi->command("gg");
    vi->command("v");
    vi->command("j");
    vi->command("%");
    REQUIRE(stc->GetSelectedText().size() == 33);
    vi->command("G");
    vi->command("v");
    vi->command("2bh");
    vi->command("%");
    // README: This should pass, but selection is not ok.
    // REQUIRE(stc->GetSelectedText().size() == 33);
  }
}
