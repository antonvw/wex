////////////////////////////////////////////////////////////////////////////////
// Name:      test-addressrange.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/addressrange.h>
#include <wex/frd.h>
#include <wex/macros.h>
#include <wex/vi.h>

#include "test.h"

TEST_CASE("wex::addressrange")
{
  const std::string contents("a\ntiger\ntiger\ntiger\ntiger\nf\ng\n");

  auto* stc = get_stc();

  stc->set_text("hello\nhello1\nhello2");

  auto* ex = new wex::vi(stc);
  ex->marker_add('x', 1);
  ex->marker_add('y', 2);
  ex->get_macros().set_register('*', "ls");
  stc->GotoLine(2);

  SUBCASE("range no selection")
  {
    REQUIRE(wex::addressrange(ex).is_ok());
    REQUIRE(wex::addressrange(ex, -1).is_ok());
    REQUIRE(wex::addressrange(ex, 5).is_ok());
    REQUIRE(wex::addressrange(ex, "%").is_ok());
    REQUIRE(wex::addressrange(ex, "*").is_ok());
    REQUIRE(wex::addressrange(ex, ".").is_ok());
    REQUIRE(wex::addressrange(ex, "1,2").is_ok());
    REQUIRE(wex::addressrange(ex, "/1/,/2/").is_ok());
    REQUIRE(wex::addressrange(ex, "?1?,?2?").is_ok());
  }

  SUBCASE("invalid range no selection")
  {
    REQUIRE(!wex::addressrange(ex, 0).is_ok());
    REQUIRE(!wex::addressrange(ex, "0").is_ok());
    REQUIRE(!wex::addressrange(ex, "x").is_ok());
    REQUIRE(!wex::addressrange(ex, "x,3").is_ok());
    REQUIRE(!wex::addressrange(ex, "x,3").erase());
    REQUIRE(!wex::addressrange(ex, "3,x").escape("ls"));
    REQUIRE(!wex::addressrange(ex, "3,x").shift_right());
    REQUIRE(!wex::addressrange(ex, "3,!").is_ok());
    REQUIRE(!wex::addressrange(ex, "3,@").move(wex::address(ex, "2")));
    REQUIRE(!wex::addressrange(ex, "1,2").move(wex::address(ex, "x")));
    REQUIRE(!wex::addressrange(ex, "1,3").move(wex::address(ex, "2")));
    REQUIRE(!wex::addressrange(ex, "3,@").copy(wex::address(ex, "2")));
    REQUIRE(!wex::addressrange(ex, "3,x").write("flut"));
    REQUIRE(!wex::addressrange(ex, " ,").yank());
    REQUIRE(!wex::addressrange(ex, "'<,'>").is_ok());
    REQUIRE(!wex::addressrange(ex, "/xx/,/2/").is_ok());
    REQUIRE(!wex::addressrange(ex, "?2?,?1?").is_ok());
  }

  SUBCASE("range selection")
  {
    stc->SelectAll();

    REQUIRE(wex::addressrange(ex, 5).is_ok());
    REQUIRE(wex::addressrange(ex, "'<,'>").is_ok());
    stc->SelectNone();
  }

  SUBCASE("invalid range selection")
  {
    stc->SelectAll();

    REQUIRE(!wex::addressrange(ex, 0).is_ok());
    REQUIRE(!wex::addressrange(ex, "0").is_ok());
    REQUIRE(!wex::addressrange(ex, "x").is_ok());
    REQUIRE(!wex::addressrange(ex, "x,3").is_ok());
    stc->SelectNone();
  }

  SUBCASE("change")
  {
    stc->set_text("a\nb\nc\nd\ne\nf\ng\n");
    REQUIRE(wex::addressrange(ex, 4).change("changed"));
    REQUIRE(stc->get_text().find("changed") != std::string::npos);
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
    REQUIRE(wex::addressrange(ex, "'<,'>").erase());
    REQUIRE(stc->get_line_count() == 1);
    stc->SelectNone();
  }

  SUBCASE("escape")
  {
    // See also del/test-frame.cpp
#ifdef __UNIX__
    stc->set_text(contents);
    REQUIRE(stc->get_line_count() == 8);
    REQUIRE(wex::addressrange(ex, "%").escape("uniq"));
    REQUIRE(stc->get_line_count() == 5);
    REQUIRE(wex::addressrange(ex, "%").escape("ls -l"));
#endif
  }

  SUBCASE("execute")
  {
    stc->set_text(contents);
    REQUIRE(!wex::addressrange(ex).execute("Z"));
  }

#ifdef __UNIX__
  SUBCASE("global")
  {
    for (bool b : {false, true})
    {
      stc->set_text(contents);
      REQUIRE(!wex::addressrange(ex, 5).global(std::string(), b));
      REQUIRE(!wex::addressrange(ex, 5).global("XXX", b));
      REQUIRE(!wex::addressrange(ex, 5).global("/", b));
      REQUIRE(wex::addressrange(ex, 5).global("/xx/p", b));
      REQUIRE(wex::addressrange(ex, 5).global("/xx/p#", b));
      REQUIRE(wex::addressrange(ex, 5).global("/xx/g", b));
      REQUIRE(wex::addressrange(ex, 5).global("/a/s/a/XX", b));
      REQUIRE(wex::addressrange(ex, 5).global("/b/s/b/XX|s/c/yy", b));
    }
  }
#endif

  SUBCASE("join")
  {
    stc->set_text("a\nb\nc\nd\ne\nf\ng\n");
    REQUIRE(wex::addressrange(ex, "%").join());
    REQUIRE(stc->get_text().find("a") != std::string::npos);
    REQUIRE(stc->get_line_count() == 1);
  }

  SUBCASE("move")
  {
    stc->set_text(contents);
    REQUIRE(stc->get_line_count() == 8);
    REQUIRE(wex::addressrange(ex, "1,2").move(wex::address(ex, "$")));
    REQUIRE(stc->get_line_count() == 8);
  }

  SUBCASE("parse")
  {
    wex::info_message_t im;

    REQUIRE(!wex::addressrange(ex, "1,3").parse("", "", im));
    REQUIRE(im == wex::info_message_t::NONE);

    REQUIRE(wex::addressrange(ex, "1,3").parse("y", "k", im));
    REQUIRE(im == wex::info_message_t::YANK);
  }

  SUBCASE("print")
  {
    stc->set_text(contents);
    REQUIRE(wex::addressrange(ex, 5).print());
  }

  SUBCASE("shift")
  {
    stc->set_text(contents);
    REQUIRE(wex::addressrange(ex, 5).shift_right());
    REQUIRE(wex::addressrange(ex, 5).shift_left());
  }

  SUBCASE("sort")
  {
    REQUIRE(wex::addressrange(ex, "1").sort());
    REQUIRE(!wex::addressrange(ex, "1").sort("x"));
    REQUIRE(wex::addressrange(ex, "1").sort("u"));
    REQUIRE(wex::addressrange(ex, "1").sort("r"));
    REQUIRE(wex::addressrange(ex, "1").sort("ur"));
  }

  SUBCASE("substitute")
  {
    stc->set_text(contents);
    stc->GotoLine(1);
    REQUIRE(wex::addressrange(ex, "%").substitute("/tiger//"));
    REQUIRE(stc->get_text().find("tiger") == std::string::npos);

    stc->set_text(contents);
    REQUIRE(wex::addressrange(ex, "%").substitute("/tiger/\\U&/"));
    REQUIRE(stc->get_text().find("TIGER") != std::string::npos);
    REQUIRE(stc->get_text().find("tiger") == std::string::npos);
    REQUIRE(stc->get_text().find("\\U") == std::string::npos);

    stc->set_text(contents);
    REQUIRE(wex::addressrange(ex, "%").substitute(
      "/tiger/\\U&&\\L& \\0 \\0 & & \\U&/"));
    REQUIRE(stc->get_text().find("TIGER") != std::string::npos);
    REQUIRE(stc->get_text().find("tiger") != std::string::npos);
    REQUIRE(stc->get_text().find("\\U") == std::string::npos);
    REQUIRE(stc->get_text().find("\\L") == std::string::npos);
    REQUIRE(stc->get_text().find("\\0") == std::string::npos);

    stc->set_text(contents);
    REQUIRE(wex::addressrange(ex, "%").substitute("/tiger/lion/"));
    REQUIRE(stc->get_text().find("lion") != std::string::npos);

    stc->set_text(contents);
    REQUIRE(wex::addressrange(ex, "%").substitute("", '&'));
    REQUIRE(stc->get_text().find("lion") != std::string::npos);

    stc->set_text(contents + " MORE");
    REQUIRE(wex::addressrange(ex, "%").substitute("", '~'));
    REQUIRE(stc->get_text().find("lion") != std::string::npos);
    REQUIRE(stc->get_text().find("tiger") == std::string::npos);

    stc->set_text("special char \\ present");
    REQUIRE(wex::addressrange(ex, "%").substitute("/\\\\//"));
    REQUIRE(stc->get_text().find("char  present") != std::string::npos);

    stc->set_text("special char / present");
    REQUIRE(wex::addressrange(ex, "%").substitute("/\\///"));
    REQUIRE(stc->get_text().find("char  present") != std::string::npos);

    stc->set_text("special char ' present");
    REQUIRE(wex::addressrange(ex, "%").substitute("/'//"));
    REQUIRE(stc->get_text().find("char  present") != std::string::npos);
  }

  SUBCASE("substitute and flags")
  {
    REQUIRE(wex::addressrange(ex, "1").substitute("//y"));
    REQUIRE(!wex::addressrange(ex, "0").substitute("/x/y"));
    REQUIRE(wex::addressrange(ex, "2").substitute("/x/y/f"));
    REQUIRE(wex::addressrange(ex, "1,2").substitute("/x/y"));
    REQUIRE(wex::addressrange(ex, "1,2").substitute("/x/y/i"));
    REQUIRE(wex::addressrange(ex, "1,2").substitute("/x/y/f"));
    REQUIRE(wex::addressrange(ex, "1,2").substitute("/x/y/g"));
    REQUIRE(wex::addressrange(ex, "1,2").substitute("g", '&'));
    REQUIRE(wex::addressrange(ex, "1,2").substitute("g", '~'));
    REQUIRE(!wex::addressrange(ex, "1,2").substitute("g", 'x'));
  }

  SUBCASE("write")
  {
    stc->set_text(contents);
    REQUIRE(wex::addressrange(ex, 5).write("sample.txt"));
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
