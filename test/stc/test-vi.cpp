////////////////////////////////////////////////////////////////////////////////
// Name:      test-vi.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/config.h>
#include <wex/core/log-none.h>
#include <wex/core/log.h>
#include <wex/ex/addressrange.h>
#include <wex/ex/ex-stream.h>
#include <wex/ui/frd.h>
#include <wex/vi/vi.h>

#include "test.h"

#define ESC "\x1b"

void change_mode(
  wex::vi*              vi,
  const std::string&    command,
  wex::vi_mode::state_t mode)
{
  REQUIRE(vi->command(command));
  REQUIRE(vi->mode().get() == mode);
}

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
  auto* stc = get_stc();
  auto* vi  = &get_stc()->get_vi();
  stc->set_text("");

  SUBCASE("find")
  {
    stc->set_text("find findnottext to another find");
    REQUIRE(vi->command("l"));
    REQUIRE(vi->command("*"));
    REQUIRE(stc->GetCurrentPos() == 32);
  }

  SUBCASE("goto") // goto, /, ?, n and N.
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
        REQUIRE(vi->command(go.first));
      else
        REQUIRE(!vi->command(go.first));

      if (go.first[0] == '/' || go.first[0] == '?')
      {
        // A / or ? should not set a last command.
        REQUIRE(vi->last_command()[0] != go.first[0]);
      }

      REQUIRE(stc->get_current_line() == go.second);
    }
  }

  SUBCASE("on_char")
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

    change_mode(vi, ESC, wex::vi_mode::state_t::COMMAND);
    REQUIRE(
      vi->inserted_text().find(vi->get_stc()->eol()) != std::string::npos);
  }

  SUBCASE("registers")
  {
    stc->get_file().file_new(wex::path("test.h"));
    const std::string ctrl_r = "\x12";
    REQUIRE(vi->command("i"));
    REQUIRE(vi->command(ctrl_r + "_"));
    change_mode(vi, ESC, wex::vi_mode::state_t::COMMAND);

    stc->set_text("");
    REQUIRE(vi->command("i"));
    REQUIRE(vi->command(ctrl_r + "%"));
    change_mode(vi, ESC, wex::vi_mode::state_t::COMMAND);
    REQUIRE(stc->get_text() == "test.h");

    REQUIRE(vi->command("yy"));
    stc->set_text("");
    REQUIRE(vi->command("i"));
    REQUIRE(vi->command(ctrl_r + "0"));
    change_mode(vi, ESC, wex::vi_mode::state_t::COMMAND);
    REQUIRE(stc->get_text() == "test.h");
  }

  SUBCASE("set")
  {
    stc->set_text("xx\nxx\nyy\nzz\n");

    REQUIRE(vi->command(":set noaw"));
    REQUIRE(vi->command(":set noic"));
    REQUIRE(vi->command(":set noreadonly"));
    REQUIRE(vi->command(":set nosws"));
    REQUIRE(vi->command(":set dir=./"));

    REQUIRE(vi->command(":set noexpandtab"));
    REQUIRE(stc->GetUseTabs());
    REQUIRE(vi->command(":set expandtab"));
    REQUIRE(!stc->GetUseTabs());

    REQUIRE(vi->command(":set report=10"));
    REQUIRE(wex::config("stc.Reported lines").get(5) == 10);

    REQUIRE(vi->command(":set ve=5"));
    REQUIRE(wex::log::get_level() == 5);

    REQUIRE(vi->command(":set ve=4"));
    REQUIRE(wex::log::get_level() == 4);
  }

#ifdef __UNIX__
  SUBCASE("source")
  {
    stc->set_text("xx\nxx\nyy\nzz\n");

    SUBCASE("so")
    {
      // necesary for the ~ in test-source
      wex::find_replace_data::get()->set_find_string("xx");

      vi->command(":so test-source.txt");
    }

    SUBCASE("full")
    {
      vi->command(":source test-source.txt");
    }

    SUBCASE("not-existing")
    {
      REQUIRE(!vi->command(":so test-surce.txt"));
    }

    SUBCASE("invalid")
    {
      // and skip the error message for recursive line
      wex::log_none off;
      REQUIRE(!vi->command(":so test-source-2.txt"));
    }

    remove("test-ex.txt");
  }
#endif

  SUBCASE("stream")
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

  SUBCASE("tab")
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
    REQUIRE(stc->get_text().find("\t") == std::string::npos);

    change_mode(vi, ESC, wex::vi_mode::state_t::COMMAND);
  }

  SUBCASE("visual")
  {
    stc->set_text("this text contains xx");

    for (const auto& visual :
         std::vector<std::pair<std::string, wex::vi_mode::state_t>>{
           {"v", wex::vi_mode::state_t::VISUAL},
           {"V", wex::vi_mode::state_t::VISUAL_LINE},
           {"K", wex::vi_mode::state_t::VISUAL_BLOCK}})
    {
      wxKeyEvent event(wxEVT_CHAR);
      change_mode(vi, visual.first, visual.second);
      change_mode(vi, "jjj", visual.second);
      change_mode(vi, visual.first, visual.second); // second has no effect
      // enter invalid command
      vi->command("g");
      vi->command("j");
      change_mode(vi, ESC, wex::vi_mode::state_t::COMMAND);

      event.m_uniChar = visual.first[0];
      REQUIRE(!vi->on_char(event));
      REQUIRE(vi->mode().get() == visual.second);
      change_mode(vi, ESC, wex::vi_mode::state_t::COMMAND);
    }
  }

  SUBCASE("others")
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
      REQUIRE(!vi->on_char(event));

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

    // Test change number.
    change_mode(vi, ESC, wex::vi_mode::state_t::COMMAND);
    event.m_uniChar = WXK_CONTROL_J;
    for (const auto& number :
         std::vector<std::string>{"101", "0xf7", "077", "-99"})
    {
      stc->set_text("number: " + number);
      vi->command("gg");
      vi->command("2w");
      REQUIRE(vi->on_key_down(event));
      REQUIRE(!vi->on_char(event));
      CAPTURE(number);
      REQUIRE(stc->get_text().find(number) == std::string::npos);
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
      change_mode(vi, ESC, wex::vi_mode::state_t::COMMAND);
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
