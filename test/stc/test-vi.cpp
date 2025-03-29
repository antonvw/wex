////////////////////////////////////////////////////////////////////////////////
// Name:      stc/test-vi.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <thread>
#include <wex/core/config.h>
#include <wex/core/log-none.h>
#include <wex/core/log.h>
#include <wex/ex/addressrange.h>
#include <wex/ex/ex-stream.h>
#include <wex/ex/util.h>
#include <wex/ui/frd.h>

#include "../ex/test-defs.h"
#include "../vi/test.h"
#include "test.h"

void create_file()
{
  const std::string text("test1\ntest2\ntest3\ntest4\n\n");
  std::fstream      ifs("ex-mode.txt", std::ios_base::out);
  REQUIRE(ifs.write(text.c_str(), text.size()));
}

wex::file open_file()
{
  create_file();

  wex::file ifs("ex-mode.txt", std::ios_base::in | std::ios_base::out);
  REQUIRE(ifs.is_open());
  return ifs;
}

TEST_CASE("wex::vi")
{
  auto* stc = new wex::stc();
  frame()->pane_add(stc);
  auto* vi = &stc->get_vi();
  stc->set_text("");

  // see also ex/test-ex.cpp
  SECTION("calculator")
  {
    stc->set_text("aaaaa\nbbbbb\nccccc\n");

    REQUIRE(vi->marker_add('a', 1));
    REQUIRE(vi->marker_add('t', 1));
    REQUIRE(vi->marker_add('u', 2));

    frame()->entry_dialog_calls_reset();

    // Only calculations that are not empty should
    // cause calling entry dialog.
    EX_CALC(vi)

    REQUIRE(frame()->entry_dialog_calls() == 4);
  }

  SECTION("change")
  {
    stc->set_text("aaaaa\nbbbbb\nccccc\naaaaa\ne\nf\ng\nh\ni\nj\nk\n");
    auto* vi = &stc->get_vi();

    SECTION("normal")
    {
      vi->command("ce");
      vi->command("OK");
      REQUIRE(
        stc->get_text() == "OK\nbbbbb\nccccc\naaaaa\ne\nf\ng\nh\ni\nj\nk\n");
    }

    SECTION("selection")
    {
      vi->mode().visual();
      vi->command("w");
      change_mode(vi, wex::esc(), wex::vi_mode::state_t::COMMAND);
      REQUIRE(stc->get_selected_text() == "aaaaa");

      vi->command("c");
      REQUIRE(vi->mode().get() == wex::vi_mode::state_t::INSERT);
      vi->command("OK");
      REQUIRE(
        stc->get_text() == "OK\nbbbbb\nccccc\naaaaa\ne\nf\ng\nh\ni\nj\nk\n");
    }

    change_mode(vi, wex::esc(), wex::vi_mode::state_t::COMMAND);
  }

  SECTION("enter")
  {
    stc->set_text("aaaaa\nbbbbb\nccccc\n");

    REQUIRE(vi->command("j\n"));
    REQUIRE(stc->get_text() == "aaaaa\nbbbbb\nccccc\n");
  }

  SECTION("find")
  {
    stc->set_text("find findnottext to another find");
    REQUIRE(vi->command("l"));
    REQUIRE(vi->command("*"));
    REQUIRE(stc->GetCurrentPos() == 32);
  }

  SECTION("goto") // goto, /, ?, n and N.
  {
    stc->set_text("aaaaa\nbbbbb\nccccc\naaaaa\ne\nf\ng\nh\ni\nj\nk\n");
    vi->reset_search_flags();
    REQUIRE(stc->get_line_count() == 12);
    stc->GotoLine(2);
    for (const auto& go : std::vector<std::pair<std::string, int>>{
           {"gg", 0},      {"G", 11},      {":-5", 6},   {":+5", 11},
           {":-", 10},     {":+", 11},     {"1G", 0},    {"10G", 9},
           {"10000G", 11}, {":$", 11},     {":100", 11}, {"/bbbbb", 1},
           {":-10", 0},    {":10", 9},     {":/c/", 2},  {":/aaaaa/", 3},
           {"://", 0},     {":10000", 11}, {":2", 1},    {"/d", 1},
           {"/a", 3},      {"n", 3},       {"N", 3},     {"?bbbbb", 1},
           {"?d", 1},      {"?a", 0},      {"n", 0},     {"N", 0}})
    {
      CAPTURE(go.first);

      if (go.first.back() != 'd')
      {
        REQUIRE(vi->command(go.first));
      }
      else
      {
        REQUIRE(!vi->command(go.first));
      }

      if (go.first[0] == '/' || go.first[0] == '?')
      {
        // A / or ? should not set a last command.
        REQUIRE(vi->last_command()[0] != go.first[0]);
      }

      REQUIRE(stc->get_current_line() == go.second);
    }
  }

  SECTION("mark")
  {
    stc->set_text("some text with marker and pos\nmore\nmore");
    REQUIRE(vi->command("2w"));
    REQUIRE(stc->GetCurrentPos() == 10);
    REQUIRE(vi->command("mx"));
    stc->DocumentEnd();
    REQUIRE(stc->get_current_line() == 2);
    REQUIRE(vi->command("'x"));
    REQUIRE(stc->get_current_line() == 0);
    REQUIRE(stc->GetCurrentPos() == 0);
    REQUIRE(vi->command("`x"));
    REQUIRE(stc->GetCurrentPos() == 10);
  }

  SECTION("navigate")
  {
    stc->set_text("{a brace and a close brace}");

    SECTION("brace")
    {
      REQUIRE(vi->command("%"));
      REQUIRE(stc->GetCurrentPos() == 26);
      REQUIRE(vi->command("%"));
      REQUIRE(stc->GetCurrentPos() == 0);
    }

    SECTION("brace-visual")
    {
      REQUIRE(vi->command("y%"));
      REQUIRE(stc->GetSelectedText().size() == 27);
    }

    SECTION("delete")
    {
      REQUIRE(vi->command(wex::k_s(WXK_DELETE)));
      REQUIRE(stc->get_text().starts_with("a brace"));
    }
  }

  SECTION("number")
  {
    change_mode(vi, wex::esc(), wex::vi_mode::state_t::COMMAND);
    wxKeyEvent event(wxEVT_CHAR);
    event.m_keyCode = WXK_CONTROL_J;
    event.m_uniChar = WXK_CONTROL_J;
    event.SetRawControlDown(true);

    for (const auto& number :
         std::vector<std::string>{"101", "0xf7", "077", "-99"})
    {
      stc->set_text("number: " + number);
      vi->command("gg");
      vi->command("2w");
      REQUIRE(vi->on_key_down(event));
      REQUIRE(!vi->on_char(event));
      CAPTURE(number);
      REQUIRE(!stc->get_text().contains(number));
    }
  }

  SECTION("on_char")
  {
    wxKeyEvent event(wxEVT_CHAR);
    event.m_uniChar = 'i';
    REQUIRE(!vi->on_char(event));
    CAPTURE(vi->mode().get());
    CAPTURE(vi->mode().str());
    CAPTURE(vi->inserted_text());
    REQUIRE(vi->mode().is_insert());
    REQUIRE(vi->inserted_text().empty());
    REQUIRE(vi->mode().is_insert());

    event.m_uniChar = WXK_RETURN;
    REQUIRE(vi->on_key_down(event));
    REQUIRE(!vi->on_char(event));

    change_mode(vi, wex::esc(), wex::vi_mode::state_t::COMMAND);
    REQUIRE(vi->inserted_text().contains(vi->get_stc()->eol()));
  }

  SECTION("put-block")
  {
    stc->set_text("XXXXX\nYYYYY  \nZZZZZ\n");

    start_block(vi);
    REQUIRE(vi->command("2j"));
    REQUIRE(vi->command(" "));
    REQUIRE(vi->command("y"));
    REQUIRE(vi->mode().get() == wex::vi_mode::state_t::COMMAND);
    REQUIRE(vi->command("h"));
    REQUIRE(vi->command("p"));

    CHECK(stc->get_text() == "XXXXXX\nYYYYYY  \nZZZZZZ\n");
  }

  SECTION("registers")
  {
    stc->get_file().file_new(wex::path("test.h"));
    const std::string ctrl_r = "\x12";

    stc->set_text("");
    REQUIRE(vi->command("i"));
    REQUIRE(vi->command(ctrl_r + "%"));
    change_mode(vi, wex::esc(), wex::vi_mode::state_t::COMMAND);
    REQUIRE(stc->get_text() == "test.h");

    REQUIRE(vi->command("yy"));
    stc->set_text("");
    REQUIRE(vi->command("i"));
    REQUIRE(vi->command(ctrl_r + "0"));
    change_mode(vi, wex::esc(), wex::vi_mode::state_t::COMMAND);
    REQUIRE(stc->get_text() == "test.h");

    stc->set_text("no control r");
    REQUIRE(vi->command("i"));
    wxKeyEvent event(wxEVT_CHAR);
    event.m_keyCode = WXK_CONTROL_R;
    event.SetControlDown(true);
    REQUIRE(vi->on_key_down(event));
    REQUIRE(!vi->on_char(event));
    change_mode(vi, wex::esc(), wex::vi_mode::state_t::COMMAND);
    REQUIRE(stc->get_text() == "no control r");
  }

  SECTION("right-while-in-insert")
  {
    stc->set_text("this text contains xx");

    wxKeyEvent event(wxEVT_CHAR);
    event.m_uniChar = 'i';
    REQUIRE(!vi->on_char(event));
    REQUIRE(vi->mode().is_insert());

    event.m_uniChar = WXK_NONE;
    event.m_keyCode = WXK_RIGHT;
    REQUIRE(vi->on_key_down(event));
    REQUIRE(!stc->get_text().contains("l"));

    change_mode(vi, wex::esc(), wex::vi_mode::state_t::COMMAND);
  }

  SECTION("select")
  {
    stc->set_text("this text contains xx");

    wxKeyEvent event(wxEVT_CHAR);
    event.m_uniChar = WXK_NONE;
    event.m_keyCode = WXK_RIGHT;

    event.SetShiftDown(true);
    REQUIRE(!vi->on_key_down(event));
    REQUIRE(vi->mode().get() == wex::vi_mode::state_t::VISUAL);
    REQUIRE(stc->get_selected_text() == "t");

    event.SetControlDown(true);
    REQUIRE(!vi->on_key_down(event));
    REQUIRE(vi->mode().get() == wex::vi_mode::state_t::VISUAL);
    REQUIRE(stc->get_selected_text() == "this ");

    event.SetShiftDown(false);
    event.SetControlDown(false);
    REQUIRE(!vi->on_key_down(event));
    REQUIRE(vi->mode().get() == wex::vi_mode::state_t::COMMAND);
    REQUIRE(stc->get_selected_text().empty());
  }

  SECTION("set")
  {
    stc->set_text("xx\nxx\nyy\nzz\n");

    // Default setting.
    REQUIRE(bool(vi->search_flags() & wxSTC_FIND_REGEXP));

    REQUIRE(vi->command(":set noaw"));
    REQUIRE(vi->command(":set noreadonly"));
    REQUIRE(vi->command(":set nosws"));
    REQUIRE(vi->command(":set dir=./"));

    // Test noic.
    REQUIRE(vi->command(":set noic"));
    REQUIRE(bool(vi->search_flags() & wxSTC_FIND_MATCHCASE));
    REQUIRE(wex::find_replace_data::get()->match_case());

    // Test mw.
    REQUIRE(vi->command(":set mw"));
    REQUIRE(bool(vi->search_flags() & wxSTC_FIND_WHOLEWORD));
    REQUIRE(wex::find_replace_data::get()->match_word());

    // Test nomagic.
    REQUIRE(vi->command(":set nomagic"));
    REQUIRE(bool(!(vi->search_flags() & wxSTC_FIND_REGEXP)));

    // And, for new component, the search_flags are kept.
    auto vi_2 = new wex::vi(stc);
    CAPTURE(vi_2->search_flags());
    REQUIRE(bool(vi_2->search_flags() & wxSTC_FIND_MATCHCASE));
    REQUIRE(bool(vi_2->search_flags() & wxSTC_FIND_WHOLEWORD));

    // Back to default.
    REQUIRE(vi->command(":set ic"));
    REQUIRE(bool(!(vi->search_flags() & wxSTC_FIND_MATCHCASE)));
    REQUIRE(!wex::find_replace_data::get()->match_case());

    REQUIRE(vi->command(":set nomw"));
    REQUIRE(bool(!(vi->search_flags() & wxSTC_FIND_WHOLEWORD)));
    REQUIRE(!wex::find_replace_data::get()->match_word());

    REQUIRE(vi->command(":set magic"));
    REQUIRE(bool(vi->search_flags() & wxSTC_FIND_REGEXP));

    REQUIRE(vi->command(":set noexpandtab"));
    REQUIRE(stc->GetUseTabs());
    REQUIRE(vi->command(":set expandtab"));
    REQUIRE(!stc->GetUseTabs());

    REQUIRE(vi->command(":set report=10"));
    REQUIRE(wex::config("stc.Reported lines").get(5) == 10);

    REQUIRE(vi->command(":set ve=5"));
    REQUIRE(std::to_underlying(wex::log::get_level()) == 5);

    REQUIRE(vi->command(":set ve=4"));
    REQUIRE(std::to_underlying(wex::log::get_level()) == 4);
  }

#ifndef __WXMSW__
  SECTION("source")
  {
    stc->set_text("xx\nxx\nyy\nzz\n");

    SECTION("so")
    {
      // necesary for the ~ in test-source
      wex::find_replace_data::get()->set_find_string("xx");

      REQUIRE(vi->command(":so test-source.txt"));
    }

    SECTION("full")
    {
      REQUIRE(vi->command(":source test-source.txt"));
    }

    SECTION("not-existing")
    {
      REQUIRE(!vi->command(":so test-surce.txt"));
    }

    SECTION("invalid")
    {
      // and skip the error message for recursive line
      wex::log_none off;
      REQUIRE(!vi->command(":so test-source-2.txt"));
    }

    remove("test-ex.txt");
  }
#endif

  SECTION("stream")
  {
    stc->set_text("\n\n\n\n\n\n");
    stc->visual(false);
    auto* vi = &stc->get_vi();

    wex::file       ifs(open_file());
    wex::ex_stream* exs(vi->ex_stream());
    ifs.open();
    exs->stream(ifs);

    wex::addressrange ar(vi, "%");
    REQUIRE(exs->get_line_count_request() == 5);
    REQUIRE(ar.begin().get_line() == 1);
    REQUIRE(ar.end().get_line() == 5);

    REQUIRE(exs->join(ar));
    REQUIRE(exs->is_modified());
    REQUIRE(exs->get_line_count() == 1);

    stc->visual(true);
  }

  SECTION("substitute")
  {
    stc->set_text("xx\nxx\nyy\nzz\n");
    REQUIRE(vi->command(":%s/$/OK"));
    REQUIRE(stc->get_text() == "xxOK\nxxOK\nyyOK\nzzOK\n");
  }

  SECTION("syntax")
  {
    REQUIRE(stc->open(wex::path("test.md")));
    stc->get_lexer().set("markdown");
    REQUIRE(stc->get_lexer().display_lexer() == "markdown");
    REQUIRE(vi->command(":syntax off"));
    REQUIRE(stc->get_lexer().display_lexer().empty());
    REQUIRE(vi->command(":syntax on"));
    REQUIRE(stc->get_lexer().display_lexer() == "markdown");
  }

  SECTION("tab")
  {
    stc->clear();

    wxKeyEvent event(wxEVT_CHAR);
    event.m_uniChar = 'i';
    REQUIRE(!vi->on_char(event));

    stc->SetUseTabs(true);
    REQUIRE(stc->GetUseTabs());

    event.m_uniChar = WXK_TAB;
    REQUIRE(vi->on_key_down(event));
    REQUIRE(!vi->on_char(event));

    REQUIRE(stc->get_text() == "\t");

    stc->clear();
    stc->SetUseTabs(false);
    REQUIRE(!stc->GetUseTabs());
    REQUIRE(vi->on_key_down(event));
    REQUIRE(!vi->on_char(event));
    REQUIRE(!stc->get_text().contains("\t"));

    change_mode(vi, wex::esc(), wex::vi_mode::state_t::COMMAND);
  }

  SECTION("visual")
  {
    stc->set_text("this text contains xx");

    for (const auto& visual : visuals())
    {
      change_mode(vi, visual.first, visual.second);
      change_mode(vi, "jjj", visual.second);
      change_mode(vi, visual.first, visual.second); // second has no effect
      // enter invalid command
      vi->command("g");
      vi->command("j");
      change_mode(vi, wex::esc(), wex::vi_mode::state_t::COMMAND);

      wxKeyEvent event(wxEVT_CHAR);
      event.m_uniChar = visual.first[0];
      REQUIRE(!vi->on_char(event));
      REQUIRE(vi->mode().get() == visual.second);
      change_mode(vi, wex::esc(), wex::vi_mode::state_t::COMMAND);
    }

    stc->set_text("this text contains xx\nand yy on other line");
    REQUIRE(stc->get_selected_text().empty());

    vi->mode().visual();
    REQUIRE(vi->mode().get() == wex::vi_mode::state_t::VISUAL);

    wxKeyEvent event(wxEVT_CHAR);
    event.m_uniChar = WXK_NONE;
    event.m_keyCode = WXK_END;

    REQUIRE(!vi->on_key_down(event));
    REQUIRE(stc->get_selected_text() == "this text contains xx");

    change_mode(vi, wex::esc(), wex::vi_mode::state_t::COMMAND);

    event.SetControlDown(true);
    REQUIRE(vi->command("gg"));
    REQUIRE(stc->get_selected_text().empty());
    vi->mode().visual();
    REQUIRE(vi->mode().get() == wex::vi_mode::state_t::VISUAL);
    REQUIRE(!vi->on_key_down(event));
    REQUIRE(
      stc->get_selected_text() ==
      "this text contains xx\nand yy on other line");
  }

  SECTION("others")
  {
    // Test WXK_NONE.
    stc->set_text("the chances of anything coming from mars\n");
    wxKeyEvent event(wxEVT_CHAR);
    event.m_uniChar = WXK_NONE;
    REQUIRE(vi->on_char(event));

    // First i enters insert mode, so is handled by vi, not to be skipped.
    event.m_uniChar = 'i';
    REQUIRE(!vi->on_char(event));
    REQUIRE(vi->mode().is_insert());
    REQUIRE(vi->mode().str() == "insert");
    // Second i (and more) all handled by vi.
    for (int i = 0; i < 10; i++)
    {
      REQUIRE(!vi->on_char(event));
    }

    // Test control keys.
    for (const auto& control_key : std::vector<int>{
           WXK_CONTROL_B,
           WXK_CONTROL_E,
           WXK_CONTROL_F,
           WXK_CONTROL_G,
           WXK_CONTROL_J,
           WXK_CONTROL_P,
           WXK_CONTROL_Q})
    {
      event.m_uniChar = control_key;
      REQUIRE(vi->on_key_down(event));
      REQUIRE(!vi->on_char(event));
    }

    // Test navigate command keys.
    for (const auto& nav_key : std::vector<int>{
           WXK_BACK,
           WXK_RETURN,
           WXK_LEFT,
           WXK_DOWN,
           WXK_UP,
           WXK_RIGHT,
           WXK_PAGEUP,
           WXK_PAGEDOWN,
           WXK_TAB})
    {
      event.m_keyCode = nav_key;
      CAPTURE(nav_key);
      change_mode(vi, wex::esc(), wex::vi_mode::state_t::COMMAND);
    }

    event.m_keyCode = WXK_NONE;
    REQUIRE(vi->on_key_down(event));

    // Test navigate with [ and ].
    event.m_uniChar = '[';
    REQUIRE(!vi->on_char(event));
    event.m_uniChar = ']';
    REQUIRE(!vi->on_char(event));
    vi->get_stc()->AppendText("{}");
    event.m_uniChar = '[';
    REQUIRE(!vi->on_char(event));
    event.m_uniChar = ']';
    REQUIRE(!vi->on_char(event));
  }

  remove("ex-mode.txt");
}
