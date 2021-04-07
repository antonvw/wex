////////////////////////////////////////////////////////////////////////////////
// Name:      test-vi.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <wex/vi.h>

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

TEST_CASE("wex::vi")
{
  auto* stc = get_stc();
  auto* vi  = &get_stc()->get_vi();
  stc->set_text("");

  SUBCASE("on_char")
  {
    wxKeyEvent event(wxEVT_CHAR);
    event.m_uniChar = 'i';
    REQUIRE(!vi->on_char(event));
    CAPTURE(vi->mode().get());
    CAPTURE(vi->mode().str());
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
    stc->get_file().file_new("test.h");
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
      // enter illegal command
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
}
