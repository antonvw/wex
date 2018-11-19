////////////////////////////////////////////////////////////////////////////////
// Name:      test-link.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/link.h>
#include <wex/config.h>
#include <wex/managedframe.h>
#include <wex/stc.h>
#include "test.h"

void link(
  const wex::link& link,
  const std::string& path, 
  const std::string& expect = std::string(),
  int expect_line_no = 0,
  int expect_col_no = 0);

void link(
  const wex::link& link,
  const std::string& path, 
  const std::string& expect,
  int expect_line_no,
  int expect_col_no)
{
  wex::control_data data;
  
  if (!expect.empty())
  {
    CAPTURE(path);
    REQUIRE(link.get_path(path, data).data().string().find(expect) != std::string::npos);
  }
  else
  {
    CAPTURE(path);
    CAPTURE(expect);
    REQUIRE(link.get_path(path, data).data().empty());
  }
  
  REQUIRE(data.line() == expect_line_no);
  REQUIRE(data.col() == expect_col_no);
}

#ifdef __UNIX__
TEST_CASE("wex::link")
{
  wex::stc* stc = get_stc();
  
  SUBCASE("Default constructor")
  {
    link(wex::link(), "xxxxx");
  }
  
  SUBCASE("Constructor with STC")
  {
    wex::link lnk(stc);  
    wex::config(_("Include directory")).set("/usr/bin");
    lnk.set_from_config();
    
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
    const std::string test("/extension/test/data/test.h");
    link(lnk, "test.h", test);
    link(lnk, "  test.h", test);
    
    const std::string special("/extension/test/data/test-special.h");
    link(lnk, "test-special.h", special);
    link(lnk, "  test-special.h", special);
    
    // Test output ls -l.
    // -rw-rw-r-- 1 anton anton  2287 nov 17 10:53 test.h
    link(lnk, "-rw-rw-r-- 1 anton anton 35278 nov 24 16:09 test.h", test);

    // Test existing file in the include path.
#ifndef __WXOSX__  
    link(lnk, "test", "/usr/bin/test");
    link(lnk, "  test \n", "/usr/bin/test"); // whitespace should be skipped
    link(lnk, "./test", "/usr/bin/./test");
    link(lnk, "<test>", "/usr/bin/test");
    link(lnk, "\"test\"", "/usr/bin/test");
    link(lnk, "skip <test> skip skip", "/usr/bin/test");
    link(lnk, "skip \"test\" skip skip", "/usr/bin/test");
    link(lnk, "skip 'test' skip skip", "/usr/bin/test");
    
    // Test incorrect line and/or col.
    link(lnk, "test:", "/usr/bin/test");
    link(lnk, "test:xyz", "/usr/bin/test");
    link(lnk, "test:50:xyz", "/usr/bin/test", 50);
    link(lnk, "test:abc:xyz", "/usr/bin/test");
    
    // Test line_no and col no.
    link(lnk, "test:50", "/usr/bin/test", 50);
    link(lnk, "test:50:", "/usr/bin/test", 50);
    link(lnk, "test:50:6", "/usr/bin/test", 50, 6);
    link(lnk, "test:500000", "/usr/bin/test", 500000);
    link(lnk, "test:500000:599", "/usr/bin/test", 500000, 599);
    link(lnk, "skip skip test:50", "/usr/bin/test", 50);

    // po file format
    link(lnk, "#: test:120", "/usr/bin/test", 120);
    stc->get_lexer().Set("po");
    link(lnk, "#: test:120", "/usr/bin/test", 120);
#endif

    link(lnk, "gcc", "/usr/bin/gcc");
    link(lnk, "<gcc>", "/usr/bin/gcc");

    link(lnk, "test-special.h:10", special, 10);
    link(lnk, "test-special.h:10:2", special, 10, 2);
  }
  
  SUBCASE("http link")
  { 
    wex::link lnk(stc);
    wex::config(_("Include directory")).set("/usr/bin");
    lnk.set_from_config();

    wex::control_data data;
    data.line(wex::link::LINE_OPEN_URL);
    REQUIRE( lnk.get_path("www.wxwidgets.org", data).data() == "www.wxwidgets.org" );
    REQUIRE( lnk.get_path("xxx.wxwidgets.org", data).data().empty());
    REQUIRE( lnk.get_path("test.cpp", data).data().empty());
    REQUIRE( lnk.get_path("<test.cpp>", data).data().empty());
    REQUIRE( lnk.get_path("gcc>", data).data().empty());
    REQUIRE( lnk.get_path("<gcc>", data).data().empty());
    REQUIRE( lnk.get_path("some text www.wxwidgets.org", data).data() == "www.wxwidgets.org" );
    REQUIRE( lnk.get_path("some text https://github.com/antonvw/wxExtension", data).data() == 
      "https://github.com/antonvw/wxExtension" );
    REQUIRE( lnk.get_path("some text (https://github.com/antonvw/wxExtension)", data).data() == 
      "https://github.com/antonvw/wxExtension" );
    REQUIRE( lnk.get_path("some text [https://github.com/antonvw/wxExtension]", data).data() == 
      "https://github.com/antonvw/wxExtension" );
    REQUIRE( lnk.get_path("httpd = new httpd", data).data().empty());

    // MIME file
    data.line(wex::link::LINE_OPEN_URL_AND_MIME);
    stc->get_file().file_new("test.html");
    REQUIRE( lnk.get_path("www.wxwidgets.org", data).data() == "www.wxwidgets.org" );
    REQUIRE( lnk.get_path("xxx.wxwidgets.org", data) == "test.html" );
    REQUIRE( lnk.get_path("xx", data).data() == "test.html" );
    data.line(-99);
    REQUIRE( lnk.get_path("xx", data).data().empty());
  }
}
#endif
