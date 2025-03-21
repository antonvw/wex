////////////////////////////////////////////////////////////////////////////////
// Name:      test-ex.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/core.h>
#include <wex/core/log-none.h>
#include <wex/ctags/ctags.h>
#include <wex/ex/ex.h>
#include <wex/ex/macros.h>

#include "test-defs.h"
#include "test.h"

// See stc/test-vi.cpp and test-ex-mode for testing goto and :set

TEST_CASE("wex::ex")
{
  auto* stc = new wex::test::stc();
  stc->visual(true);
  auto* ex = new wex::ex(stc);
  stc->SetReadOnly(false);
  stc->set_text("xx\nxx\nyy\nzz\n");
  stc->DocumentStart();

  const wex::path p("test.h");
  ALLOW_CALL(*stc, path()).RETURN(p);

  SUBCASE("abbreviations")
  {
    REQUIRE(ex->command(":ab t TTTT"));
    const auto& it1 = ex->get_macros().get_abbreviations().find("t");
    REQUIRE(it1 != ex->get_macros().get_abbreviations().end());
    REQUIRE(it1->second == "TTTT");
    REQUIRE(ex->command(":una t"));
    REQUIRE(!ex->command(":unabbrv t"));
    REQUIRE(!ex->command(":unabbrevv t"));
    REQUIRE(ex->command(":unabbrev t"));
    REQUIRE(
      ex->get_macros().get_abbreviations().find("t") ==
      ex->get_macros().get_abbreviations().end());
  }

  SUBCASE("calculator")
  {
    stc->set_text("aaaaa\nbbbbb\nccccc\n");

    REQUIRE(ex->marker_add('a', 1));
    REQUIRE(ex->marker_add('t', 1));
    REQUIRE(ex->marker_add('u', 2));

    EX_CALC(ex)
  }

#ifdef __UNIX__
  SUBCASE("cd")
  {
    wex::path keep;

    REQUIRE(ex->command(":chd"));
    CAPTURE(keep.string());
    REQUIRE(keep.original() != wex::path::current().data());

    for (const auto& command : std::vector<std::pair<std::string, std::string>>{
           {":chd /usr", "/usr"},
           {":chd .", "/usr"},
           {":chd ..", "/"},
           {":chdir /usr", "/usr"},
           {":chdir .", "/usr"},
           {":chdir ..", "/"},
           {":cd /usr", "/usr"},
           {":cd .", "/usr"},
           {":cd ..", "/"}})
    {
      CAPTURE(command.first);
      REQUIRE(ex->command(command.first));
      REQUIRE(wex::path::current().string() == command.second);
    }
  }
#endif

  SUBCASE("commands")
  {
    // Most commands are tested using the :so command in stc/test-vi.cpp
    for (const auto& command :
         std::vector<std::string>{":ab", ":ve", ":vi", ":1,$s/s/w/"})
    {
      CAPTURE(command);
      REQUIRE(ex->command(command));
      REQUIRE(ex->get_command().command().empty());
    }
  }

  SUBCASE("ctags")
  {
    REQUIRE(ex->ctags() != nullptr);
    REQUIRE(wex::ctags::find("test_app"));
    REQUIRE(!wex::ctags::find("xest_app"));
  }

  SUBCASE("edit")
  {
    REQUIRE(ex->command(":e test.txt"));
  }

  SUBCASE("general")
  {
    REQUIRE(ex->frame() == frame());
    REQUIRE(!ex->get_macros().mode().is_recording());
    REQUIRE(ex->ex_stream() != nullptr);
    ex->info_message("hello world", wex::info_message_t::ADD);
  }

  SUBCASE("global")
  {
    // Test global delete (previous delete was on found text).
    const int max = 10;
    for (int i = 0; i < max; i++)
    {
      stc->AppendText("line xxxx added\n");
    }
    const int lines = stc->get_line_count();
    REQUIRE(ex->command(":g/xxxx/d"));
    REQUIRE(stc->get_line_count() == lines - max);

    stc->AppendText("line xxxx 6 added\n");
    stc->AppendText("line xxxx 7 added\n");
    REQUIRE(ex->command(":g/xxxx/s//yyyy"));
    REQUIRE(stc->get_text().contains("yyyy"));
    REQUIRE(ex->command(":g//"));

    // Test global move.
    stc->set_text("a\nb\nc\nd\ne\nf\ng\nh\ni\nj\nk\n");
    REQUIRE(ex->command(":g/d/m$")); // possible infinite loop
    REQUIRE(stc->get_text().contains("d"));

    stc->set_text("a\nb\nc\nd\ne\nf\ng\nh\ni\nj\nk\n");
    REQUIRE(ex->command(":g/d/p"));
    REQUIRE(ex->command(":g//d"));
    REQUIRE(ex->command(":g//"));
    REQUIRE(ex->command(":g//p"));
  }

  SUBCASE("input-mode")
  {
    const std::string eol(ex->get_stc()->eol());
    REQUIRE(ex->command(":a|added"));
    CAPTURE(stc->get_text());
    REQUIRE(stc->get_text().contains("xx\nadded" + eol + "xx"));

    REQUIRE(ex->command(":i|inserted"));
    CAPTURE(stc->get_text());
    REQUIRE(
      stc->get_text().find("inserted" + eol + "added" + eol + "xx") !=
      std::string::npos);

    REQUIRE(ex->command(":c|changed"));
    REQUIRE(stc->get_text().contains("changed"));

    const int lines = stc->get_line_count();

    REQUIRE(ex->command(":1,2co$"));
    REQUIRE(stc->get_line_count() == lines + 2);
    REQUIRE(ex->command(":1,2copy$"));
    REQUIRE(stc->get_line_count() == lines + 4);
    REQUIRE(ex->command(":1,2t$"));
    REQUIRE(stc->get_line_count() == lines + 6);

    REQUIRE(ex->command(":1,2nu"));
    REQUIRE(ex->command(":1,2number"));
    REQUIRE(!ex->command(":1,2nuber"));
    REQUIRE(ex->command(":1,2#"));
  }

  SUBCASE("invalid-commands")
  {
    ex->get_stc()->add_text("XXX");

    for (const auto& command : std::vector<std::string>{
           // We have only one document, so :n, :prev return false.
           ":n",
           ":axx",
           ":ixx",
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
           ":/XXX/y",
           ":r test-xx.txt"})
    {
      CAPTURE(command);
      REQUIRE(!ex->command(command));
    }
  }

  SUBCASE("inverse")
  {
    SUBCASE("example")
    {
      stc->set_text("xx0\n"
                    "xx1\n"
                    "yy2\n"
                    "xx3\n"
                    "yy4\n"
                    "yy5\n"
                    "yy6\n"
                    "yy7\n"
                    "yy8\n"
                    "yy9\n"
                    "xx10\n"
                    "xx11\n"
                    "yy12\n"
                    "yy13\n"
                    "pp14\n");

      REQUIRE(ex->command(":v/yy/d"));
      REQUIRE(stc->get_line_count() == 9);
      REQUIRE(!stc->get_text().contains("xx"));
      REQUIRE(!stc->get_text().contains("pp"));
    }

    SUBCASE("extra")
    {
      stc->set_text("");

      const int max = 10;
      for (int i = 0; i < max; i++)
      {
        stc->AppendText("line xxxx added\n");
        stc->AppendText("line yyyy added\n");
      }

      REQUIRE(ex->command(":v/xxxx/d"));
      REQUIRE(stc->get_line_count() == max);
      REQUIRE(!stc->get_text().contains("yy"));
    }
  }

  SUBCASE("is_active")
  {
    REQUIRE(ex->is_active());
    ex->use(wex::ex::mode_t::OFF);
    REQUIRE(!ex->is_active());
    ex->use(wex::ex::mode_t::VISUAL);
    REQUIRE(ex->is_active());
    REQUIRE(ex->visual() == wex::ex::mode_t::VISUAL);
  }

  SUBCASE("line-data")
  {
    REQUIRE(!ex->line_data().is_ctag());

    wex::line_data data;
    data.is_ctag(true);
    ex->set_line_data(data);
    REQUIRE(ex->line_data().is_ctag());
  }

  SUBCASE("map")
  {
    REQUIRE(ex->command(":map :xx :%d"));
    REQUIRE(ex->command(":xx"));
    REQUIRE(stc->get_text().empty());
    REQUIRE(ex->command(":unm xx"));
  }

  SUBCASE("marker_and_register_expansion")
  {
    stc->set_text("this is some text");
    REQUIRE(ex->command(":ky"));

    std::string command("xxx");
    REQUIRE(!wex::marker_and_register_expansion(nullptr, command));
    REQUIRE(wex::marker_and_register_expansion(ex, command));
    REQUIRE(command == "xxx");

    command = "'yxxx";
    REQUIRE(wex::marker_and_register_expansion(ex, command));
    REQUIRE(command == "1xxx");

    command = "`yxxx";
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

  SUBCASE("markers")
  {
    REQUIRE(ex->marker_add('a'));
    REQUIRE(ex->marker_line('a') != -1);
    REQUIRE(!ex->marker_goto("a"));
    REQUIRE(ex->marker_goto("\'a"));
    REQUIRE(!ex->marker_goto("\' "));
    REQUIRE(ex->marker_delete('a'));
    REQUIRE(!ex->marker_delete('b'));
    REQUIRE(!ex->marker_goto("\'a"));
    REQUIRE(!ex->marker_delete('a'));
    stc->set_text("xx\nyy\nzz\n");
    stc->goto_line(0);
    REQUIRE(ex->marker_add('t'));
    stc->goto_line(stc->get_line_count() - 1);
    REQUIRE(ex->marker_add('u'));
    REQUIRE(ex->command(":'t,'us/s/w/"));
    REQUIRE(ex->command(":'t,$s/s/w/"));
    REQUIRE(ex->command(":1,'us/s/w/"));

    wex::log_none off;
    REQUIRE(!ex->marker_add(' '));
  }

  SUBCASE("print")
  {
    ex->print("This is printed");
    REQUIRE(ex->get_print_text() == "This is printed");

    stc->set_text("this is some text\nnext line");
    REQUIRE(ex->command(":p"));
    REQUIRE(ex->get_print_text() == "this is some text\n");
    REQUIRE(ex->command(":%p"));
    REQUIRE(ex->get_print_text() == "this is some text\nnext line\n");
    REQUIRE(ex->command(":l"));
    REQUIRE(ex->get_print_text() == "this is some text$\n");
    REQUIRE(ex->command(":%l"));
    REQUIRE(ex->get_print_text() == "this is some text$\nnext line$\n");
  }

  SUBCASE("range")
  {
    for (const auto& cmd : std::vector<std::string>{
           "<",
           ">",
           "co1",
           "copy1",
           "t1",
           "move1",
           "m1",
           "ya",
           "yank",
#ifndef __WXMSW__
           "!sort",
#endif
           "w text-ex.txt",
           "write text-ex.txt"})
    {
      stc->GotoLine(2);
      stc->LineDownExtend();
      CAPTURE(cmd);
      REQUIRE(ex->command(":" + wex::ex_command::selection_range() + cmd));
    }

    REQUIRE(remove("text-ex.txt") == 0);

    stc->GotoLine(2);
    stc->LineDownExtend();
    REQUIRE(!ex->command(":" + wex::ex_command::selection_range() + "y"));

    stc->set_text("blame\n\ncopy chances");
    REQUIRE(ex->command(":/blame/,/copy/ya"));
    REQUIRE(ex->command(":/blame/,/y/ya"));

    REQUIRE(ex->command(":1,2>"));

    stc->SelectNone();
    REQUIRE(!ex->command(":" + wex::ex_command::selection_range() + ">>"));
  }

  SUBCASE("read")
  {
#ifdef __UNIX__
    REQUIRE(ex->command(":r !echo qwerty"));
    REQUIRE(stc->get_text().contains("qwerty"));
#endif
  }

  SUBCASE("registers")
  {
    wex::ex::set_registers_delete("x");
    wex::ex::set_register_yank("test");
    REQUIRE(ex->get_macros().get_register('0') == "test");
    REQUIRE(ex->register_text() == "test");
    wex::ex::set_register_insert("insert");
    REQUIRE(wex::ex::register_insert() == "insert");

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

  SUBCASE("search_flags")
  {
    REQUIRE((ex->search_flags() & wxSTC_FIND_REGEXP) > 0);
  }

  SUBCASE("substitute")
  {
    stc->set_text("we have ccccc yyyy zzzz");

    SUBCASE("eol")
    {
      REQUIRE(ex->command(":%s/z$/z>"));
      REQUIRE(stc->get_text() == "we have ccccc yyyy zzzz>");
    }

    SUBCASE("regular")
    {
      REQUIRE(ex->command(":%s/ccccc/ddd"));
      REQUIRE(stc->get_text() == "we have ddd yyyy zzzz");
      stc->set_text("we have xxxx yyyy zzzz");
      ex->reset_search_flags();
      REQUIRE(ex->command(":%s/(x+) *(y+)/\\\\2 \\\\1"));
      REQUIRE(stc->get_text() == "we have yyyy xxxx zzzz");
      stc->set_text("we have 'x'xxx 'zzzz'");
      REQUIRE(ex->command(":%s/'//g"));
      REQUIRE(stc->get_text() == "we have xxxx zzzz");
      REQUIRE(!ex->command(":.s/x*//g"));
      REQUIRE(!ex->command(":.s/ *//g"));
    }

    SUBCASE("tilde")
    {
      stc->set_text("we have xxxxx yyyyy zzzzz");
      REQUIRE(ex->command(":%s/x+/vvvvv/"));
      REQUIRE(stc->get_text() == "we have vvvvv yyyyy zzzzz");

      // tilde for replacement
      stc->set_text("we have xxxxx yyyyy zzzzz");
      REQUIRE(ex->command(":%s/y+/~"));
      REQUIRE(stc->get_text() == "we have xxxxx vvvvv zzzzz");

      // tilde for target and replacement
      stc->set_text("we have xxxxx yyyyy zzzzz");
      REQUIRE(ex->command(":%s/~"));
      REQUIRE(stc->get_text() == "we have xxxxx vvvvv zzzzz");

      // tilde for complete last subtitute
      stc->set_text("we have xxxxx yyyyy zzzzz");
      REQUIRE(ex->command(":~"));
      REQUIRE(stc->get_text() == "we have xxxxx vvvvv zzzzz");
    }
  }

  SUBCASE("text-input")
  {
    const std::string eol(ex->get_stc()->eol());
    stc->set_text("xyz\n");
    REQUIRE(ex->command(":append|extra"));
    REQUIRE(stc->get_text() == "xyz\nextra" + eol);
    REQUIRE(ex->command(":insert|before\n"));
    REQUIRE(stc->get_text() == "xyz\nbefore\n" + eol + "extra" + eol);
    stc->set_text("xyz\n");
    REQUIRE(ex->command(":c|new"));
    REQUIRE(stc->get_text() == "new\n");
  }

  SUBCASE("yank")
  {
    stc->set_text("xyz\n");
    REQUIRE(ex->command(":1,5ya"));
    REQUIRE(ex->command(":1,5yank"));
    REQUIRE(ex->command(":1,5yank c"));
    REQUIRE(wex::ex::get_macros().get_register('c').contains("xyz"));

    REQUIRE(!ex->command(":1,5yb"));
    REQUIRE(!ex->command(":1,5yankc"));
  }
}
