////////////////////////////////////////////////////////////////////////////////
// Name:      test-vi-other.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/config.h>
#include <wex/core/core.h>
#include <wex/ex/macro-mode.h>
#include <wex/ex/macros.h>
#include <wex/ex/util.h>

#include "../ex/test.h"
#include "test.h"

TEST_CASE("wex::vi-other")
{
  auto* stc = get_stc();
  auto* vi  = new wex::vi(get_stc());

  // First load macros.
  REQUIRE(wex::ex::get_macros().load_document());

  SECTION("join")
  {
    stc->set_text("this text contains xx\nand yy");
    REQUIRE(stc->get_line_count() == 2);
    REQUIRE(stc->get_current_line() == 0);

    REQUIRE(vi->command("J"));
    REQUIRE(stc->get_line_count() == 1);
  }

  SECTION("macro")
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

  SECTION("playback")
  {
    REQUIRE(vi->command("qa"));

    REQUIRE(vi->command("atest"));
    change_mode(vi, wex::esc(), wex::vi_mode::state_t::COMMAND);

    REQUIRE(vi->command("q"));

    stc->set_text("");
    REQUIRE(vi->command("@a"));
    REQUIRE(stc->get_text() == "test");
  }

  // this subcase should be before the 'recording', otherwise it fails?
  SECTION("replace")
  {
    stc->set_text("XXXXX\nYYYYY\nZZZZZ\n");

    SECTION("normal")
    {
      REQUIRE(vi->command("3rx"));
      REQUIRE(stc->get_text() == "xxxXX\nYYYYY\nZZZZZ\n");
    }

    SECTION("block")
    {
      REQUIRE(vi->command("K"));
      REQUIRE(vi->command("j"));
      REQUIRE(vi->command("j"));
      REQUIRE(vi->command("3rx"));
      REQUIRE(stc->get_text() == "xxxXX\nxxxYY\nxxxZZ\n");
      change_mode(vi, wex::esc(), wex::vi_mode::state_t::COMMAND);
    }
  }

  SECTION("recording")
  {
    stc->set_text("abc\nuvw\nxyz\n");

    REQUIRE(!wex::ex::get_macros().mode().is_recording());
    vi->command("");
    REQUIRE(vi->command("qa"));
    REQUIRE(wex::ex::get_macros().mode().is_recording());
    REQUIRE(vi->command("/abc"));
    REQUIRE(vi->command("q"));
    REQUIRE(!wex::ex::get_macros().mode().is_recording());
    REQUIRE(!wex::ex::get_macros().get_macro_commands("a").empty());
    REQUIRE(vi->command("@a"));
    REQUIRE(vi->command(" "));
    REQUIRE(vi->command("qb"));
    REQUIRE(wex::ex::get_macros().mode().is_recording());
    REQUIRE(vi->command("?abc"));
    REQUIRE(vi->command("q"));
    REQUIRE(!wex::ex::get_macros().mode().is_recording());
    REQUIRE(vi->command("@b"));
    REQUIRE(vi->command(" "));
    REQUIRE(vi->command("qc"));
    REQUIRE(vi->command("\"x"));
    REQUIRE(vi->command("yw"));
    REQUIRE(vi->command("q"));
  }

  SECTION("variable")
  {
    stc->set_text("");
    REQUIRE(vi->command("@Date@"));
    REQUIRE(!stc->get_text().contains("Date"));

    stc->set_text("");
    REQUIRE(vi->command("@Year@"));
    REQUIRE(stc->get_text().contains("20"));
    REQUIRE(stc->get_lexer().set("cpp"));

    stc->set_text("");
    REQUIRE(vi->command("@Cb@"));
    REQUIRE(vi->command("@Ce@"));
    REQUIRE(stc->get_text().contains("//"));

    stc->set_text("");
    REQUIRE(vi->command("@Cl@"));
    REQUIRE(stc->get_text().contains("//"));
    REQUIRE(vi->command("@Nl@"));
  }

  SECTION("yank")
  {
    stc->set_text("xxxxxxxxxx second\nxxxxxxxx\naaaaaaaaaa\n");
    REQUIRE(vi->command("yw"));
    REQUIRE(vi->get_stc()->get_selected_text() == "xxxxxxxxxx ");
    REQUIRE(vi->command("w"));
    REQUIRE(vi->command("x"));
    REQUIRE(stc->get_text().contains("second"));
    REQUIRE(vi->command("p"));
    REQUIRE(stc->get_text().contains("second"));
  }

  SECTION("other")
  {
    // Test abbreviate.
    for (auto& abbrev : wex::test::get_abbreviations())
    {
      REQUIRE(vi->command(":ab " + abbrev.first + " " + abbrev.second));
      REQUIRE(vi->command("iabbreviation " + abbrev.first + " "));
      change_mode(vi, wex::esc(), wex::vi_mode::state_t::COMMAND);
      REQUIRE(vi->inserted_text() == "abbreviation " + abbrev.first + " ");
      REQUIRE(stc->get_text().contains("abbreviation " + abbrev.second));
      REQUIRE(!stc->get_text().contains(abbrev.first));
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
      {
        continue;
      }

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
          {
            REQUIRE(vi->command(std::string(1, c)));
          }
          else
          {
            REQUIRE(!vi->command(std::string(1, c)));
          }
        }
      }
      else
      {
        const auto oc(
          other_command.first == "m" || other_command.first == "q" ||
              other_command.first == "r" || other_command.first == "\x12" ||
              other_command.first == "@" ?
            other_command.first + "a" :
            other_command.first);
        CAPTURE(oc);

        if (oc != "z")
        {
          REQUIRE(vi->command(oc));
        }
        else
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

  delete vi;
}
