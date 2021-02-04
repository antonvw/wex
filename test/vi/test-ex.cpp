////////////////////////////////////////////////////////////////////////////////
// Name:      test-ex.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <vector>

#include "../test.h"
#include <wex/core.h>
#include <wex/ex.h>
#include <wex/frd.h>
#include <wex/macro-mode.h>
#include <wex/macros.h>
#include <wex/managed-frame.h>
#include <wex/path.h>
#include <wex/stc.h>

TEST_SUITE_BEGIN("wex::ex");

TEST_CASE("wex::ex")
{
  SUBCASE("modeline")
  {
    SUBCASE("text")
    {
      const std::string modeline("set ts=120 ec=40 sy=sql sw=4 nu el");
      auto*             stc = new wex::stc(std::string("-- vi: " + modeline));
      wex::test::add_pane(frame(), stc);

      REQUIRE(stc->get_vi().is_active());
      REQUIRE(stc->GetTabWidth() == 120);
      REQUIRE(stc->GetEdgeColumn() == 40);
      REQUIRE(stc->GetIndent() == 4);
      REQUIRE(stc->get_lexer().scintilla_lexer() == "sql");
    }

    SUBCASE("head")
    {
      auto* stc = new wex::stc(wex::path("test-modeline.txt"));
      wex::test::add_pane(frame(), stc);

      auto* timer = new wxTimer(frame());
      timer->StartOnce(1000);
      frame()->Bind(wxEVT_TIMER, [=](wxTimerEvent& event) {
        REQUIRE(stc->get_lexer().scintilla_lexer() == "sql");
      });
    }

    SUBCASE("tail")
    {
      auto* stc = new wex::stc(wex::path("test-modeline2.txt"));
      wex::test::add_pane(frame(), stc);

      auto* timer = new wxTimer(frame());
      timer->StartOnce(1000);
      frame()->Bind(wxEVT_TIMER, [=](wxTimerEvent& event) {
        REQUIRE(stc->get_lexer().scintilla_lexer() == "sql");
      });
    }
  }

  wex::stc* stc = get_stc();
  wex::ex*  ex  = &stc->get_ex();
  stc->set_text("xx\nxx\nyy\nzz\n");
  stc->DocumentStart();

  SUBCASE("general")
  {
    REQUIRE(ex->frame() == frame());
    REQUIRE(!ex->get_macros().mode().is_recording());
  }

  SUBCASE("input mode")
  {
    REQUIRE(ex->command(":a|added"));
    REQUIRE(stc->get_text().find("added") != std::string::npos);

    REQUIRE(ex->command(":i|inserted"));
    REQUIRE(stc->get_text().find("inserted") != std::string::npos);

    REQUIRE(ex->command(":c|changed"));
    REQUIRE(stc->get_text().find("changed") != std::string::npos);
  }

  SUBCASE("is_active")
  {
    // currently the get_ex returns the get_vi
    REQUIRE(ex->is_active());
    ex->use(false);
    REQUIRE(!ex->is_active());
    ex->use(true);
    REQUIRE(ex->is_active());
  }

  SUBCASE("marker_and_register_expansion")
  {
    stc->set_text("this is some text");
    REQUIRE(ex->command(":ky"));

    std::string command("xxx");
    REQUIRE(!wex::marker_and_register_expansion(nullptr, command));
    REQUIRE(wex::marker_and_register_expansion(ex, command));

    command = "'yxxx";
    REQUIRE(wex::marker_and_register_expansion(ex, command));
    REQUIRE(command == "1xxx");

    command = "yxxx'";
    REQUIRE(wex::marker_and_register_expansion(ex, command));
    REQUIRE(command == "yxxx'");

    REQUIRE(wex::clipboard_add("yanked"));
    command = "this is * end";
    REQUIRE(wex::marker_and_register_expansion(ex, command));

#ifndef __WXMSW__
    REQUIRE(command == "this is yanked end");
#endif
  }

  SUBCASE("visual mode")
  {
    ex->get_stc()->visual(true);
    REQUIRE(!ex->get_stc()->data().flags().test(wex::data::stc::WIN_EX));
    REQUIRE(ex->is_active()); // vi

    ex->get_stc()->visual(false);
    CAPTURE(ex->get_stc()->data().flags());
    REQUIRE(ex->get_stc()->data().flags().test(wex::data::stc::WIN_EX));
    REQUIRE(!ex->is_active()); // vi

    REQUIRE(ex->get_stc()->get_ex().command(":vi"));
    REQUIRE(!ex->get_stc()->data().flags().test(wex::data::stc::WIN_EX));
    REQUIRE(ex->is_active()); // vi
  }

  SUBCASE("search_flags")
  {
    REQUIRE((ex->search_flags() & wxSTC_FIND_REGEXP) > 0);
  }

  SUBCASE("commands")
  {
    // Most commands are tested using the :so command.
    for (const auto& command : std::vector<std::pair<std::string, bool>>{
           {":ab", true},
           {":ve", false},
           {":1,$s/s/w/", true}})
    {
      CAPTURE(command);
      REQUIRE(ex->command(command.first));
    }
  }

  SUBCASE("invalid commands")
  {
    ex->get_stc()->add_text("XXX");

    for (const auto& command : std::vector<std::string>{
           // We have only one document, so :n, :prev return false.
           ":n",
           ":prev",
           ":.k",
           ":pk",
           ":.pk",
           ":so",
           ":so xxx",
           ":vix",
           ":xxx",
           ":zzz",
           ":%/test//",
           ":1,$k",
           ":.S0",
           ":.Sx",
           ":/XXX/x",
           ":r test-xx.txt"})
    {
      CAPTURE(command);
      REQUIRE(!ex->command(command));
    }
  }

  SUBCASE("map")
  {
    REQUIRE(ex->command(":map :xx :%d"));
    REQUIRE(ex->command(":xx"));
    REQUIRE(stc->get_text().empty());
    REQUIRE(ex->command(":unm xx"));
  }

  SUBCASE("text input")
  {
    stc->set_text("xyz\n");
    REQUIRE(ex->command(":append|extra"));
    REQUIRE(stc->get_text() == "xyz\nextra");
    REQUIRE(ex->command(":insert|before\n"));
    REQUIRE(stc->get_text() == "xyz\nbefore\nextra");
    stc->set_text("xyz\n");
    REQUIRE(ex->command(":c|new\n"));
    REQUIRE(stc->get_text() == "new\n");
  }

  SUBCASE("abbreviations")
  {
    REQUIRE(ex->command(":ab t TTTT"));
    const auto& it1 = ex->get_macros().get_abbreviations().find("t");
    REQUIRE(it1 != ex->get_macros().get_abbreviations().end());
    REQUIRE(it1->second == "TTTT");
    REQUIRE(ex->command(":una t"));
    REQUIRE(
      ex->get_macros().get_abbreviations().find("t") ==
      ex->get_macros().get_abbreviations().end());
  }

  SUBCASE("range")
  {
    REQUIRE(ex->command(":1,2>"));

    stc->SelectNone();
    REQUIRE(!ex->command(":'<,'>>"));

    stc->GotoLine(2);
    stc->LineDownExtend();
    REQUIRE(ex->command(":'<,'>m1"));

    stc->GotoLine(2);
    stc->LineDownExtend();
    stc->LineDownExtend();
    stc->LineDownExtend();
    REQUIRE(ex->command(":'<,'>w test-ex.txt"));
    REQUIRE(ex->command(":'<,'><"));

    ex->command(":'<,'>>");

#ifndef __WXMSW__
    ex->command(":'<,'>!sort");
#endif

    stc->GotoLine(2);
    stc->LineDownExtend();
    REQUIRE(!ex->command(":'<,'>x"));
  }

  SUBCASE("read")
  {
#ifdef __UNIX__
    REQUIRE(ex->command(":r !echo qwerty"));
    REQUIRE(stc->get_text().find("qwerty") != std::string::npos);
#endif
  }

#ifdef __UNIX__
  SUBCASE("source")
  {
    SUBCASE("so")
    {
      REQUIRE(stc->find(std::string("xx")));
      REQUIRE(
        stc->get_find_string() == "xx"); // necesary for the ~ in test-source
      REQUIRE(ex->command(":so test-source.txt"));
    }

    SUBCASE("full") { REQUIRE(ex->command(":source test-source.txt")); }

    SUBCASE("not-existing") { REQUIRE(!ex->command(":so test-surce.txt")); }

    SUBCASE("illegal") { REQUIRE(!ex->command(":so test-source-2.txt")); }
  }
#endif

  SUBCASE("markers")
  {
    REQUIRE(ex->marker_add('a'));
    REQUIRE(ex->marker_line('a') != -1);
    REQUIRE(ex->marker_goto('a'));
    REQUIRE(ex->marker_delete('a'));
    REQUIRE(!ex->marker_delete('b'));
    REQUIRE(!ex->marker_goto('a'));
    REQUIRE(!ex->marker_delete('a'));
    stc->set_text("xx\nyy\nzz\n");
    REQUIRE(ex->command(":1"));
    REQUIRE(ex->marker_add('t'));
    REQUIRE(ex->command(":$"));
    REQUIRE(ex->marker_add('u'));
    REQUIRE(ex->command(":'t,'us/s/w/"));
    REQUIRE(ex->command(":'t,$s/s/w/"));
    REQUIRE(ex->command(":1,'us/s/w/"));
  }

  SUBCASE("print") { ex->print("This is printed"); }

  SUBCASE("global")
  {
    // Test global delete (previous delete was on found text).
    const int max = 10;
    for (int i = 0; i < max; i++)
      stc->AppendText("line xxxx added\n");
    const int lines = stc->get_line_count();
    REQUIRE(ex->command(":g/xxxx/d"));
    REQUIRE(stc->get_line_count() == lines - max);

    stc->AppendText("line xxxx 6 added\n");
    stc->AppendText("line xxxx 7 added\n");
    REQUIRE(ex->command(":g/xxxx/s//yyyy"));
    REQUIRE(stc->get_text().find("yyyy") != std::string::npos);
    REQUIRE(ex->command(":g//"));

    // Test global move.
    stc->set_text("a\nb\nc\nd\ne\nf\ng\nh\ni\nj\nk\n");
    REQUIRE(!ex->command(":g/d/m$")); // possible infinite loop
    REQUIRE(stc->get_text().find("d") != std::string::npos);
  }

  SUBCASE("substitute")
  {
    stc->set_text("we have ccccc yyyy zzzz");
    REQUIRE(ex->command(":%s/ccccc/ddd"));
    REQUIRE(stc->get_text() == "we have ddd yyyy zzzz");
    stc->set_text("we have xxxx yyyy zzzz");
    ex->reset_search_flags();
    REQUIRE(ex->command(":%s/(x+) *(y+)/\\\\2 \\\\1"));
    REQUIRE(stc->get_text() == "we have yyyy xxxx zzzz");
    stc->set_text("we have xxxx 'zzzz'");
    REQUIRE(ex->command(":%s/'//g"));
    REQUIRE(stc->get_text() == "we have xxxx zzzz");
    REQUIRE(!ex->command(":.s/x*//g"));
    REQUIRE(!ex->command(":.s/ *//g"));
  }

  SUBCASE("goto")
  {
    stc->set_text("a\nb\nc\nd\ne\nf\ng\nh\ni\nj\nk\n");
    REQUIRE(stc->get_line_count() == 12);
    stc->GotoLine(2);

    for (const auto& go : std::vector<std::pair<std::string, int>>{
           {":1", 0},
           {":-10", 0},
           {":10", 9},
           {":/c/", 2},
           {":10000", 11}})
    {
      REQUIRE(ex->command(go.first));
      REQUIRE(stc->get_current_line() == go.second);
    }
  }

  SUBCASE("registers")
  {
    ex->set_registers_delete("x");
    ex->set_register_yank("test");
    REQUIRE(ex->get_macros().get_register('0') == "test");
    REQUIRE(ex->register_text() == "test");
    ex->set_register_insert("insert");
    REQUIRE(ex->register_insert() == "insert");

    stc->set_text("the chances");
    stc->SelectAll();
    REQUIRE(ex->yank());
    stc->SelectNone();
    REQUIRE(!ex->yank());

    REQUIRE(ex->register_text() == "the chances");
    stc->SelectAll();
    ex->cut();
    REQUIRE(ex->register_text() == "the chances");
    REQUIRE(ex->get_macros().get_register('1') == "the chances");
    REQUIRE(ex->get_stc()->get_selected_text().empty());
  }

  SUBCASE("calculator")
  {
    stc->set_text("aaaaa\nbbbbb\nccccc\n");

    REQUIRE(ex->marker_add('a', 1));
    REQUIRE(ex->marker_add('t', 1));
    REQUIRE(ex->marker_add('u', 2));

    std::vector<std::pair<std::string, std::pair<double, int>>> calcs{
      {"", {0, 0}},      {"  ", {0, 0}},    {"1 + 1", {2, 0}},
      {"5+5", {10, 0}},  {"1 * 1", {1, 0}}, {"1 - 1", {0, 0}},
      {"2 / 1", {2, 0}}, {"2 / 0", {0, 0}}, {"2 < 2", {8, 0}},
      {"2 > 1", {1, 0}}, {"2 | 1", {3, 0}}, {"2 & 1", {0, 0}},
      {"~0", {-1, 0}},   {"4 % 3", {1, 0}}, {".", {1, 0}},
      {"xxx", {0, 0}},   {"%s", {0, 0}},    {"%s/xx/", {0, 0}},
      {"'a", {2, 0}},    {"'t", {2, 0}},    {"'u", {3, 0}},
      {"$", {4, 0}}};

    for (const auto& calc : calcs)
    {
      const auto val = ex->calculator(calc.first);
      REQUIRE(val == calc.second.first);
    }
  }
}

TEST_SUITE_END();
