////////////////////////////////////////////////////////////////////////////////
// Name:      test-link.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/link.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/stc.h>
#include "test.h"

//#define DEBUG ON

void link(
    const wxExLink& link,
    const wxString& path, 
    const wxString& expect = wxEmptyString,
    int expect_line_no = 0,
    int expect_col_no = 0);

void link(
  const wxExLink& link,
  const wxString& path, 
  const wxString& expect,
  int expect_line_no,
  int expect_col_no)
{
  int line_no = 0;
  int col_no = 0;

#ifdef DEBUG  
  wxLogMessage("in: %s out: %s expect: %s %d %d\n", 
    path.c_str(), link.GetPath(path, line_no, col_no).c_str(), expect.c_str(), 
    expect_line_no, expect_col_no);
#endif

  if (!expect.empty())
  {
    REQUIRE(link.GetPath(path, line_no, col_no).Contains(expect));
  }
  else
  {
    REQUIRE(link.GetPath(path, line_no, col_no) == expect);
  }
  
  REQUIRE(line_no == expect_line_no);
  REQUIRE(col_no == expect_col_no);
}

TEST_CASE("wxExLink", "[stc]")
{
  wxExSTC* stc = new wxExSTC(
    GetFrame(), 
    "hello stc, \"X-Poedit-Basepath: /usr/bin\\n\"");
  AddPane(GetFrame(), stc);
  
  wxExLink lnk(stc);  
  
  REQUIRE( lnk.AddBasePath());
  REQUIRE( lnk.AddBasePath());
  
  const wxString test("/extension/test/data/test.h");
  const wxString special("/extension/test/data/test-special.h");
  
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
  link(lnk, "test.h", test);
  link(lnk, "  test.h", test);
  link(lnk, "test-special.h", special);
  link(lnk, "  test-special.h", special);
  
  // Test output ls -l.
  // -rw-rw-r-- 1 anton anton  2287 nov 17 10:53 test.h
  link(lnk, "-rw-rw-r-- 1 anton anton 35278 nov 24 16:09 test.h", test);

  // Test existing file in the basepath.
  link(lnk, "test", "/usr/bin/test");
  link(lnk, "  test \n", "/usr/bin/test"); // whitespace should be skipped
  link(lnk, "./test", "/usr/bin/./test");
  link(lnk, "<test>", "/usr/bin/test");
  link(lnk, "\"test\"", "/usr/bin/test");
  link(lnk, "skip <test> skip skip", "/usr/bin/test");
  link(lnk, "skip \"test\" skip skip", "/usr/bin/test");
  link(lnk, "skip 'test' skip skip", "/usr/bin/test");
  
  // Test existing file in the basepath, incorrect line and/or col.
  link(lnk, "test:", "/usr/bin/test");
  link(lnk, "test:xyz", "/usr/bin/test");
  link(lnk, "test:50:xyz", "/usr/bin/test", 50);
  link(lnk, "test:abc:xyz", "/usr/bin/test");
  
  // Test existing file, line_no and col no.
  link(lnk, "test:50", "/usr/bin/test", 50);
  link(lnk, "test:50:", "/usr/bin/test", 50);
  link(lnk, "test:50:6", "/usr/bin/test", 50, 6);
  link(lnk, "test:500000", "/usr/bin/test", 500000);
  link(lnk, "test:500000:599", "/usr/bin/test", 500000, 599);
  link(lnk, "skip skip test:50", "/usr/bin/test", 50);
  link(lnk, "test-special.h:10", special, 10);
  link(lnk, "test-special.h:10:2", special, 10, 2);
  // po file format
  link(lnk, "#: test:120", "/usr/bin/test", 120);
  
  lnk.SetFromConfig();
  
  // Now we have no basepath, so previous test is different.
  link(lnk, "test");
  
  // Test link with default contructor.
  wxExLink lnk2;
  
  REQUIRE(!lnk2.AddBasePath());
  REQUIRE(!lnk2.AddBasePath());
  
  link(lnk2, "test");
}
