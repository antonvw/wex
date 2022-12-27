////////////////////////////////////////////////////////////////////////////////
// Name:      test-addressrange.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/ex/addressrange.h>
#include <wex/ex/command-parser.h>
#include <wex/ex/ex.h>
#include <wex/ex/macros.h>

#include "test.h"

TEST_CASE("wex::addressrange")
{
  const std::string contents("a\nTIGER\ntiger\ntiger\ntiger\nf\ng\n");

  auto* stc = get_stc();

  stc->set_text("hello\nhello11\nhello22\ntest\ngcc\nblame\nthis\nyank\ncopy");

  auto* ex = new wex::ex(stc);
  ex->marker_add('x', 1);
  ex->marker_add('y', 2);
  ex->get_macros().set_register('*', "ls");
  stc->GotoLine(2);

  SUBCASE("constructor")
  {
    SUBCASE("default")
    {
      wex::addressrange ar(ex);

      REQUIRE(ar.begin().get_line() == 3);
      REQUIRE(ar.end().get_line() == 3);
      REQUIRE(!ar.find_indicator().is_ok());
    }

    SUBCASE("range-int")
    {
      wex::addressrange ar(ex, 6);

      REQUIRE(ar.begin().get_line() == 3);
      REQUIRE(ar.end().get_line() == 8);
      REQUIRE(!ar.find_indicator().is_ok());
    }

    SUBCASE("range-string")
    {
      wex::addressrange ar(ex, "");

      REQUIRE(ar.begin().get_line() == 0);
      REQUIRE(ar.end().get_line() == 0);
      REQUIRE(!ar.find_indicator().is_ok());
    }
  }

  SUBCASE("change")
  {
    stc->set_text("a\nb\nc\nd\ne\nf\ng\n");
    REQUIRE(ex->command(":1,4c|changed"));
    REQUIRE(stc->get_text().contains("changed"));
  }

  SUBCASE("copy")
  {
    stc->set_text(contents);
    REQUIRE(stc->get_line_count() == 8);
    REQUIRE(wex::addressrange(ex, "1,2").copy(wex::address(ex, "$")));
    REQUIRE(stc->get_line_count() == 10);
  }

  SUBCASE("erase")
  {
    REQUIRE(wex::addressrange(ex, "1,3").erase());
    REQUIRE(wex::addressrange(ex, "1,3").erase());

    stc->set_text("a\nb\nc\nd\ne\nf\ng\n");
    stc->GotoLine(1);
    REQUIRE(wex::addressrange(ex, 5).erase());
    REQUIRE(stc->get_line_count() == 3);
    REQUIRE(!wex::addressrange(ex, 0).erase());
    REQUIRE(stc->get_line_count() == 3);
    stc->set_text("a\nb\nc\nd\ne\nf\ng\n");
    stc->SelectAll();
    REQUIRE(wex::addressrange(ex, wex::ex_command::selection_range()).erase());
    REQUIRE(stc->get_line_count() == 1);
    stc->SelectNone();
  }

  SUBCASE("escape")
  {
    // See also del/test-frame.cpp
#ifdef __UNIX__
    stc->set_text(contents);
    REQUIRE(stc->get_line_count() == 8);
    REQUIRE(ex->command(":%!uniq"));
    REQUIRE(stc->get_line_count() == 6);
    REQUIRE(ex->command(":%!ls -l"));
#endif
  }

  SUBCASE("execute")
  {
    stc->set_text(contents);
    REQUIRE(!ex->command("1,5@Z"));
  }

#ifdef __UNIX__
  SUBCASE("global")
  {
    stc->set_text(contents);
    REQUIRE(!ex->command(":1,5g"));
    REQUIRE(!ex->command(":1,5gXXX"));
    REQUIRE(!ex->command(":1,5g/"));
    REQUIRE(ex->command(":1,5g/xx/p"));
    REQUIRE(ex->command(":1,5v/xx/p"));
    REQUIRE(ex->command(":1,5g/xx/p#"));
    REQUIRE(ex->command(":1,5global/xx/p#"));
    REQUIRE(ex->command(":1,5v/xx/p#"));
    REQUIRE(ex->command(":1,5g/xx/g"));
    REQUIRE(ex->command(":1,5g/a/s/a/XX"));
    REQUIRE(ex->command(":1,5g/b/s/b/XX|s/c/yy"));
  }
#endif

  SUBCASE("invalid-range")
  {
    REQUIRE(!wex::addressrange(ex, 0).is_ok());
    REQUIRE(!wex::addressrange(ex, "0").is_ok());
    REQUIRE(!wex::addressrange(ex, "x").is_ok());
    REQUIRE(!wex::addressrange(ex, "x,3").is_ok());
    REQUIRE(!wex::addressrange(ex, "x,3").erase());
    REQUIRE(!wex::addressrange(ex, "3,x").shift_right());
    REQUIRE(!wex::addressrange(ex, "3,!").is_ok());
    REQUIRE(!wex::addressrange(ex, "3,@").copy(wex::address(ex, "2")));
    REQUIRE(!wex::addressrange(ex, "1,2").copy(wex::address(ex, "x")));
    REQUIRE(!wex::addressrange(ex, "1,3").copy(wex::address(ex, "2")));
    REQUIRE(!wex::addressrange(ex, "3,@").copy(wex::address(ex, "2")));
    REQUIRE(!wex::addressrange(ex, " ,").yank());
    REQUIRE(!wex::addressrange(ex, wex::ex_command::selection_range()).is_ok());
    REQUIRE(!wex::addressrange(ex, "/xx/,/2/").is_ok());
    REQUIRE(!wex::addressrange(ex, "?2?,?1?").is_ok());
  }

  SUBCASE("invalid-range-selection")
  {
    stc->SelectAll();

    REQUIRE(!wex::addressrange(ex, 0).is_ok());
    REQUIRE(!wex::addressrange(ex, "0").is_ok());
    REQUIRE(!wex::addressrange(ex, "x").is_ok());
    REQUIRE(!wex::addressrange(ex, "x,3").is_ok());
    stc->SelectNone();
  }

  SUBCASE("join")
  {
    stc->set_text("a\nb\nc\nd\ne\nf\ng\n");
    REQUIRE(wex::addressrange(ex, "%").join());
    REQUIRE(stc->get_text().contains("a"));
    REQUIRE(stc->get_line_count() == 1);
  }

  SUBCASE("move")
  {
    stc->set_text(contents);
    REQUIRE(stc->get_line_count() == 8);
    REQUIRE(ex->command(":1,2m$"));
    REQUIRE(stc->get_line_count() == 8);
  }

  SUBCASE("parse")
  {
    wex::info_message_t im;

    REQUIRE(!wex::addressrange(ex).parse(wex::command_parser(ex, "1,3"), im));
    REQUIRE(im == wex::info_message_t::NONE);

    wex::command_parser cp(ex, "1,3ya k");

    REQUIRE(wex::addressrange(ex).parse(cp, im));
    REQUIRE(im == wex::info_message_t::YANK);
  }

  SUBCASE("print")
  {
    stc->set_text(contents);
    REQUIRE(ex->command(":1,5print"));
  }

  SUBCASE("range")
  {
    REQUIRE(wex::addressrange(ex).is_ok());
    REQUIRE(wex::addressrange(ex, -1).is_ok());
    REQUIRE(wex::addressrange(ex, 5).is_ok());
    REQUIRE(wex::addressrange(ex, "%").is_ok());
    REQUIRE(wex::addressrange(ex, "*").is_ok());
    REQUIRE(wex::addressrange(ex, ".").is_ok());
    REQUIRE(wex::addressrange(ex, "1,2").is_ok());
    REQUIRE(wex::addressrange(ex, "/1/,/2/").is_ok());
    REQUIRE(wex::addressrange(ex, "/11/,/22/").is_ok());
    REQUIRE(wex::addressrange(ex, "/test/,/gcc/").is_ok());
    REQUIRE(wex::addressrange(ex, "/blame/,/copy/").is_ok());
    REQUIRE(wex::addressrange(ex, "/blame/,/this/").is_ok());
    REQUIRE(wex::addressrange(ex, "/blame/,/yank/").is_ok());
    REQUIRE(wex::addressrange(ex, "?1?,?2?").is_ok());
    REQUIRE(!wex::addressrange(ex, wex::ex_command::selection_range()).is_ok());

    wex::addressrange ar(ex, "/blame/,/yank/");
    REQUIRE(ar.is_ok());
    REQUIRE(ar.begin().type() == wex::address::IS_BEGIN);
    REQUIRE(ar.end().type() == wex::address::IS_END);
  }

  SUBCASE("range-selection")
  {
    stc->SelectAll();

    wex::addressrange range(ex, wex::ex_command::selection_range());
    REQUIRE(range.is_ok());
    REQUIRE(range.is_selection());
    REQUIRE(wex::addressrange(ex, 5).is_ok());

    stc->SelectNone();
  }

  SUBCASE("shift")
  {
    stc->set_text(contents);
    REQUIRE(wex::addressrange(ex, 5).shift_right());
    REQUIRE(wex::addressrange(ex, 5).shift_left());
  }

  SUBCASE("sort")
  {
    REQUIRE(ex->command(":1,2S"));
    REQUIRE(!ex->command(":1,2Sx"));
    REQUIRE(ex->command(":1,2Su"));
    REQUIRE(ex->command(":1,2Sr"));
    REQUIRE(ex->command(":1,2Sur"));
  }

  SUBCASE("substitute")
  {
    stc->set_text(contents);
    REQUIRE(ex->command(":%s/tiger//"));
    REQUIRE(!stc->get_text().contains("tiger"));

    stc->set_text(contents);
    REQUIRE(ex->command(":%s/tiger/\\U&/g"));
    REQUIRE(stc->get_text().contains("TIGER"));
    REQUIRE(!stc->get_text().contains("tiger"));
    REQUIRE(!stc->get_text().contains("\\U"));

    stc->set_text(contents);
    REQUIRE(ex->command(":%s/tiger/\\U&&\\L& \\0 \\0 & & \\U&/"));
    REQUIRE(stc->get_text().contains("TIGER"));
    REQUIRE(stc->get_text().contains("tiger"));
    REQUIRE(!stc->get_text().contains("\\U"));
    REQUIRE(!stc->get_text().contains("\\L"));
    REQUIRE(!stc->get_text().contains("\\0"));

    stc->set_text(contents);
    REQUIRE(ex->command(":%s/tiger/lion/"));
    REQUIRE(stc->get_text().contains("lion"));

    stc->set_text(contents);
    REQUIRE(ex->command(":%&"));
    REQUIRE(stc->get_text().contains("lion"));

    stc->set_text(contents + " MORE");
    REQUIRE(ex->command(":%~"));
    REQUIRE(stc->get_text().contains("lion"));
    REQUIRE(!stc->get_text().contains("tiger"));

    stc->set_text("special char \\ present");
    REQUIRE(ex->command(":%s/\\\\//"));
    REQUIRE(stc->get_text().contains("char  present"));

    stc->set_text("special char / present");
    REQUIRE(ex->command(":%s/\\///"));
    REQUIRE(stc->get_text().contains("char  present"));

    stc->set_text("special char ' present");
    REQUIRE(ex->command(":%s/'//"));
    REQUIRE(stc->get_text().contains("char  present"));
  }

  SUBCASE("substitute-flags")
  {
    REQUIRE(ex->command(":.,.+1s//y"));
    REQUIRE(ex->command(":.,.+2s/x/y/f"));
    REQUIRE(ex->command(":1,2s/x/y"));
    REQUIRE(ex->command(":1,2s/x/y/i"));
    REQUIRE(ex->command(":1,2s/x/y/f"));
    REQUIRE(ex->command(":1,2s/x/y/g"));
    REQUIRE(ex->command(":1,2&g"));
    REQUIRE(ex->command(":1,2~g"));
    REQUIRE(!ex->command(":1,2sxg"));
  }

  SUBCASE("write")
  {
    stc->set_text(contents);
    REQUIRE(ex->command(":1,5w sample.txt"));
    REQUIRE(remove("sample.txt") == 0);
  }

  SUBCASE("yank")
  {
    stc->set_text("a\nb\nc\nd\ne\nf\ng\n");
    stc->GotoLine(0);
    REQUIRE(wex::addressrange(ex, 2).yank());
    stc->SelectNone();
    stc->add_text(ex->get_macros().get_register('0'));
    REQUIRE(stc->get_line_count() == 10);
    REQUIRE(wex::addressrange(ex, -2).erase());
    stc->GotoLine(0);
    REQUIRE(wex::addressrange(ex, -2).erase());
  }
}
