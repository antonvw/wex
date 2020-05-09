////////////////////////////////////////////////////////////////////////////////
// Name:      test-ex.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/version.hpp>
#include <vector>

#include "../test.h"
#include <wex/ex.h>
#include <wex/frd.h>
#include <wex/macro-mode.h>
#include <wex/macros.h>
#include <wex/managedframe.h>
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
  wex::ex*  ex  = &stc->get_vi();
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
    REQUIRE(stc->GetText().find("added") != std::string::npos);

    REQUIRE(ex->command(":i|inserted"));
    REQUIRE(stc->GetText().find("inserted") != std::string::npos);

    REQUIRE(ex->command(":c|changed"));
    REQUIRE(stc->GetText().find("changed") != std::string::npos);
  }

  SUBCASE("is_active")
  {
    REQUIRE(ex->is_active());
    ex->use(false);
    REQUIRE(!ex->is_active());
    ex->use(true);
    REQUIRE(ex->is_active());
  }

  SUBCASE("search_flags")
  {
    REQUIRE((ex->search_flags() & wxSTC_FIND_REGEXP) > 0);
  }

  SUBCASE("test commands")
  {
    // Most commands are tested using the :so command.
    for (const auto& command :
         std::vector<std::pair<std::string, bool>>{{":ab", true},
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
    stc->set_text("123456789");
    REQUIRE(ex->command(":map :xx :%d"));
    REQUIRE(ex->command(":xx"));
    REQUIRE(stc->GetText().empty());
    REQUIRE(ex->command(":unm xx"));
  }

  SUBCASE("text input")
  {
    stc->set_text("xyz\n");
    REQUIRE(ex->command(":append|extra"));
    REQUIRE(stc->GetText() == "xyz\nextra");
    REQUIRE(ex->command(":insert|before\n"));
    REQUIRE(stc->GetText() == "xyz\nbefore\nextra");
    stc->set_text("xyz\n");
    REQUIRE(ex->command(":c|new\n"));
    REQUIRE(stc->GetText() == "new\n");
  }

  SUBCASE("abbreviations")
  {
    stc->set_text("xx\n");
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
    stc->set_text("a\nb\nc\nd\ne\nf\ng\nh\ni\nj\nk\n");
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

#if BOOST_VERSION / 100 % 1000 != 72
    ex->command(":'<,'>!sort");
#endif

    stc->GotoLine(2);
    stc->LineDownExtend();
    REQUIRE(!ex->command(":'<,'>x"));
  }

  SUBCASE("source")
  {
#ifdef __UNIX__
    stc->set_text("xx\nxx\nyy\nzz\n");
    REQUIRE(ex->command(":so test-source.txt"));
    stc->set_text("xx\nxx\nyy\nzz\n");
    REQUIRE(ex->command(":source test-source.txt"));
    stc->set_text("xx\nxx\nyy\nzz\n");
    REQUIRE(!ex->command(":so test-surce.txt"));
    stc->set_text("xx\nxx\nyy\nzz\n");
    REQUIRE(!ex->command(":so test-source-2.txt"));
    REQUIRE(ex->command(":d"));

#if BOOST_VERSION / 100 % 1000 != 72
    REQUIRE(ex->command(":r !echo qwerty"));
    REQUIRE(stc->GetText().Contains("qwerty"));
#endif
#endif
  }

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
    const int lines = stc->GetLineCount();
    REQUIRE(ex->command(":g/xxxx/d"));
    REQUIRE(stc->GetLineCount() == lines - max);

    stc->AppendText("line xxxx 6 added\n");
    stc->AppendText("line xxxx 7 added\n");
    REQUIRE(ex->command(":g/xxxx/s//yyyy"));
    REQUIRE(stc->GetText().Contains("yyyy"));
    REQUIRE(ex->command(":g//"));

    // Test global move.
    stc->set_text("a\nb\nc\nd\ne\nf\ng\nh\ni\nj\nk\n");
    REQUIRE(!ex->command(":g/d/m$")); // possible infinite loop
    REQUIRE(stc->GetText().Contains("d"));
  }

  SUBCASE("substitute")
  {
    stc->set_text("we have ccccc yyyy zzzz");
    REQUIRE(ex->command(":%s/ccccc/ddd"));
    REQUIRE(stc->GetText() == "we have ddd yyyy zzzz");
    stc->set_text("we have xxxx yyyy zzzz");
    ex->reset_search_flags();
    REQUIRE(ex->command(":%s/(x+) *(y+)/\\\\2 \\\\1"));
    REQUIRE(stc->GetText() == "we have yyyy xxxx zzzz");
    stc->set_text("we have xxxx 'zzzz'");
    REQUIRE(ex->command(":%s/'//g"));
    REQUIRE(stc->GetText() == "we have xxxx zzzz");
    REQUIRE(!ex->command(":.s/x*//g"));
    REQUIRE(!ex->command(":.s/ *//g"));
  }

  SUBCASE("goto")
  {
    stc->set_text("a\nb\nc\nd\ne\nf\ng\nh\ni\nj\nk\n");
    REQUIRE(stc->GetLineCount() == 12);
    stc->GotoLine(2);

    for (const auto& go :
         std::vector<std::pair<std::string, int>>{{":1", 0},
                                                  {":-10", 0},
                                                  {":10", 9},
                                                  {":/c/", 2},
                                                  {":10000", 11}})
    {
      REQUIRE(ex->command(go.first));
      REQUIRE(stc->GetCurrentLine() == go.second);
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
