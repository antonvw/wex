////////////////////////////////////////////////////////////////////////////////
// Name:      test-cmdlineparser.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/numformatter.h>
#include <wx/extension/cmdline.h>
#include "../test.h"

TEST_CASE("wxExCmdLine")
{
  int a;
  float b;
  bool s, t, u, v;
  std::string c,p,q,r;
  const wxChar ds(wxNumberFormatter::GetDecimalSeparator());
  
  const std::string str(ds == '.' ?
    "wxExCmdLine -a 10 -b 5.1 -c test -s -t -u -v one two three":
    "wxExCmdLine -a 10 -b 5.1 -c test -s -t -u -v one two three");
  
  REQUIRE( wxExCmdLine(
     {{{"s", "1", "bool"}, [&](bool on){s = on;}},
      {{"t", "2", "bool"}, [&](bool on){t = on;}},
      {{"u", "3", "bool"}, [&](bool on){u = true;}},
      {{"v", "4", "bool"}, [&](bool on){v = on;}}},
     {{{"a", "5", "int"}, {CMD_LINE_INT, [&](const wxAny& i) {a = i.As<int>();}}},
      {{"b", "6", "float"}, {CMD_LINE_FLOAT, [&](const wxAny& i) {b = i.As<float>();}}},
      {{"c", "7", "string"}, {CMD_LINE_STRING, [&](const wxAny& s) {c = s.As<std::string>();}}}},
     {{"rest", "rest"}, [&](const std::vector<std::string> & v) {
        p = v[0];
        q = v[1];
        r = v[2];
        return true;}}).Parse(str));

  REQUIRE( a == 10 );
  REQUIRE( b == 5.1f );
  REQUIRE( c == "test" );
  REQUIRE( s );
  REQUIRE( t );
  REQUIRE( u );
  REQUIRE( v );
  REQUIRE( p == "one" );
  REQUIRE( q == "two" );
  REQUIRE( r == "three" );
}
