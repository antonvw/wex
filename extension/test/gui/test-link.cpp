////////////////////////////////////////////////////////////////////////////////
// Name:      test-link.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/extension/link.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/stc.h>
#include "test.h"

void link(
  const wxExLink& link,
  const std::string& path, 
  const std::string& expect = std::string(),
  int expect_line_no = 0,
  int expect_col_no = 0);

void link(
  const wxExLink& link,
  const std::string& path, 
  const std::string& expect,
  int expect_line_no,
  int expect_col_no)
{
  wxExControlData data;
  
  if (!expect.empty())
  {
    REQUIRE(link.GetPath(path, data).find(expect) != std::string::npos);
  }
  else
  {
    REQUIRE(link.GetPath(path, data).empty());
  }
  
  REQUIRE(data.Line() == expect_line_no);
  REQUIRE(data.Col() == expect_col_no);
}

#ifdef __UNIX__
TEST_CASE("wxExLink")
{
  wxExSTC* stc = GetSTC();
  
  SUBCASE("Default constructor")
  {
    const wxExLink lnk;
    link(lnk, "test");
  }
  
  SUBCASE("Constructor with STC")
  {
    wxExLink lnk(stc);  
    wxConfigBase::Get()->Write(_("Include directory"), "/usr/bin");
    lnk.SetFromConfig();
    
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
    stc->GetLexer().Set("po");
    link(lnk, "#: test:120", "/usr/bin/test", 120);
#endif

    link(lnk, "gcc", "/usr/bin/gcc");
    link(lnk, "<gcc>", "/usr/bin/gcc");

    link(lnk, "test-special.h:10", special, 10);
    link(lnk, "test-special.h:10:2", special, 10, 2);
  }
  
  SUBCASE("http link")
  { 
    wxExLink lnk(stc);
    wxConfigBase::Get()->Write(_("Include directory"), "/usr/bin");
    lnk.SetFromConfig();

    wxExControlData data;
    REQUIRE( lnk.GetPath("xxx.wxwidgets.org", data).empty());
    REQUIRE( lnk.GetPath("<test.cpp>", data).empty());
    data.Line(-1);
    REQUIRE( lnk.GetPath("gcc>", data).empty());
    REQUIRE( lnk.GetPath("<gcc>", data).empty());
    REQUIRE( lnk.GetPath("xxx.wxwidgets.org", data).empty());
    REQUIRE( lnk.GetPath("xxx.wxwidgets.org", data).empty());
    REQUIRE( lnk.GetPath("<test.cpp>", data).empty());
    REQUIRE( lnk.GetPath("www.wxwidgets.org", data) == "www.wxwidgets.org" );
    REQUIRE( lnk.GetPath("some text www.wxwidgets.org", data) == "www.wxwidgets.org" );
    REQUIRE( lnk.GetPath("some text https://github.com/antonvw/wxExtension", data) == "https://github.com/antonvw/wxExtension" );
    REQUIRE( lnk.GetPath("some text (https://github.com/antonvw/wxExtension)", data) == "https://github.com/antonvw/wxExtension" );
    REQUIRE( lnk.GetPath("httpd = new httpd", data).empty());
    data.Line(0);
    REQUIRE( lnk.GetPath("some text https://github.com/antonvw/wxExtension", data).empty() );
    // hypertext file
    stc->GetFile().FileNew("test.html");
    REQUIRE( lnk.GetPath("www.wxwidgets.org", data).empty() );
    REQUIRE( lnk.GetPath("xx", data).empty());
    data.Line(-1);
    REQUIRE( lnk.GetPath("xx", data) == "test.html" );
    data.Line(-2);
    REQUIRE( lnk.GetPath("xx", data).empty());
  }
}
#endif
