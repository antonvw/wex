////////////////////////////////////////////////////////////////////////////////
// Name:      test-link.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/config.h>
#include <wex/link.h>

#include "test.h"

void link(
  const wex::link&   link,
  const std::string& path,
  const std::string& expect         = std::string(),
  int                expect_line_no = 0,
  int                expect_col_no  = 0);

void link(
  const wex::link&   link,
  const std::string& path,
  const std::string& expect,
  int                expect_line_no,
  int                expect_col_no)
{
  wex::data::control data;

  if (!expect.empty())
  {
    CAPTURE(path);
    CAPTURE(expect);
    const std::string p(link.get_path(path, data).string());
    CAPTURE(p);
    REQUIRE(p.find(expect) != std::string::npos);
  }
  else
  {
    CAPTURE(path);
    REQUIRE(link.get_path(path, data).empty());
  }

  REQUIRE(data.line() == expect_line_no);
  REQUIRE(data.col() == expect_col_no);
}

#ifdef __UNIX__
TEST_CASE("wex::link")
{
  auto* stc = get_stc();

  SUBCASE("constructor")
  {
    wex::config(_("stc.link.Include directory"))
      .set(std::list<std::string>{{"/usr/bin"}});
    wex::link lnk;

    // Test empty, or illegal paths.
    link(lnk, "");
    link(lnk, "xxxx");
    link(lnk, "1 othertest:");
    link(lnk, ":test");
    link(lnk, ": xtest");
    link(lnk, "c:test");
    link(lnk, "c:\\test");
    link(lnk, "on xxxx: 1200");
    link(lnk, "on xxxx: not a number");

    // Test existing file in test data dir.
    const std::string test("/test/data/test.h");
    link(lnk, "test.h", test);
    link(lnk, "  test.h", test);

    const std::string special("/test/data/test-special.h");
    link(lnk, "test-special.h", special);
    link(lnk, "  test-special.h", special);

    // Test output ls -l.
    // -rw-rw-r-- 1 anton anton  2287 nov 17 10:53 test.h
    link(lnk, "-rw-rw-r-- 1 anton anton 35278 nov 24 16:09 test.h", test);

    // Test existing file in the include path.
    link(lnk, "who", "/usr/bin/who");
    link(lnk, "  who \n", "/usr/bin/who"); // whitespace should be skipped
    link(lnk, "./who", "/usr/bin/./who");

    // who incorrect line and/or col.
    link(lnk, "who:", "/usr/bin/who");
    link(lnk, "who:xyz", "/usr/bin/who");
    link(lnk, "who:50:xyz", "/usr/bin/who", 50);
    link(lnk, "who:abc:xyz", "/usr/bin/who");

    // who line_no and col no.
    link(lnk, "who:50", "/usr/bin/who", 50);
    link(lnk, "who:50:", "/usr/bin/who", 50);
    link(lnk, "who:50:6", "/usr/bin/who", 50, 6);
    link(lnk, "who:500000", "/usr/bin/who", 500000);
    link(lnk, "who:500000:599", "/usr/bin/who", 500000, 599);
    link(lnk, "skip skip who:50", "/usr/bin/who", 50);

    // po file format
    link(lnk, "#: who:120", "/usr/bin/who", 120);
    link(lnk, "#: who:120", "/usr/bin/who", 120);

    link(lnk, "gcc", "/usr/bin/gcc");

    link(lnk, "test-special.h:10", special, 10);
    link(lnk, "test-special.h:10:2", special, 10, 2);
  }

  SUBCASE("http link")
  {
    wex::link lnk;
    wex::config(_("stc.link.Include directory"))
      .set(std::list<std::string>{{"/usr/bin"}});
    lnk.config_get();

    wex::data::control data;
    data.line(wex::link::LINE_OPEN_URL);
    REQUIRE(
      lnk.get_path("www.wxwidgets.org", data).data() == "www.wxwidgets.org");
    REQUIRE(lnk.get_path("xxx.wxwidgets.org", data).empty());
    REQUIRE(lnk.get_path("test.cpp", data).empty());
    REQUIRE(lnk.get_path("<test.cpp>", data).empty());
    REQUIRE(lnk.get_path("gcc>", data).empty());
    REQUIRE(lnk.get_path("<gcc>", data).empty());
    REQUIRE(
      lnk.get_path("some text www.wxwidgets.org", data).data() ==
      "www.wxwidgets.org");
    REQUIRE(
      lnk.get_path("some text https://github.com/wxWidgets", data).data() ==
      "https://github.com/wxWidgets");
    REQUIRE(
      lnk.get_path("some text (https://github.com/wxWidgets)", data).data() ==
      "https://github.com/wxWidgets");
    REQUIRE(
      lnk.get_path("some text [https://github.com/wxWidgets]", data).data() ==
      "https://github.com/wxWidgets");
    REQUIRE(lnk.get_path("httpd = new httpd", data).empty());

    // MIME file
    data.line(wex::link::LINE_OPEN_MIME);
    stc->get_file().file_new("test.html");
    REQUIRE(lnk.get_path("www.wxwidgets.org", data).data().empty());
    REQUIRE(lnk.get_path("xxx.wxwidgets.org", data, stc) == "test.html");
    REQUIRE(lnk.get_path("xx", data, stc).data() == "test.html");

    data.line(-99);
    REQUIRE(lnk.get_path("xx", data, stc).empty());
  }
}
#endif
