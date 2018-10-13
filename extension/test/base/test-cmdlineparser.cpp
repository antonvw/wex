////////////////////////////////////////////////////////////////////////////////
// Name:      test-cmdlineparser.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/numformatter.h>
#include <wx/extension/cmdline.h>
#include "../test.h"

TEST_CASE("wex::cmdline")
{
  int a;
  float b;
  bool s, t, u, v, x;
  std::string c,p,q,r;
  const wxChar ds(wxNumberFormatter::GetDecimalSeparator());
  
  const std::string str(ds == '.' ?
    "wex::cmdline -a 10 -b 5.1 -c test -s -t -u -v --xx one two three":
    "wex::cmdline -a 10 -b 5.1 -c test -s -t -u -v --xx one two three");

  wex::cmdline cmdl(
     {{{"s", "1", "bool"}, [&](bool on){s = on;}},
      {{"t", "2", "bool"}, [&](bool on){t = on;}},
      {{"u", "3", "bool"}, [&](bool on){u = true;}},
      {{"v", "4", "bool"}, [&](bool on){v = on;}},
      {{"xx", "bool"}, [&](bool on){x = on;}}},
     {{{"a", "5", "int"}, {wex::CMD_LINE_INT, [&](const std::any& i) {a = std::any_cast<int>(i);}}},
      {{"b", "6", "float"}, {wex::CMD_LINE_FLOAT, [&](const std::any& f) {b = std::any_cast<float>(f);}}},
      {{"c", "7", "string"}, {wex::CMD_LINE_STRING, [&](const std::any& s) {c = std::any_cast<std::string>(s);}}}},
     {{"rest", "rest"}, [&](const std::vector<std::string> & v) {
        p = v[0];
        q = v[1];
        r = v[2];
        return true;}});
  
  REQUIRE( cmdl.Parse(str));
  REQUIRE( cmdl.Delimiter() == ' ');

  cmdl.Delimiter('x');
  REQUIRE( cmdl.Delimiter() == 'x');

  REQUIRE( a == 10 );
  REQUIRE( b == 5.1f );
  REQUIRE( c == "test" );
  REQUIRE( s );
  REQUIRE( t );
  REQUIRE( u );
  REQUIRE( v );
  REQUIRE( x );
  REQUIRE( p == "one" );
  REQUIRE( q == "two" );
  REQUIRE( r == "three" );
}
