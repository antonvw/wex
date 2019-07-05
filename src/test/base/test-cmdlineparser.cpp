////////////////////////////////////////////////////////////////////////////////
// Name:      test-cmdlineparser.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/cmdline.h>
#include "../test.h"

TEST_CASE("wex::cmdline")
{
  int a {0};
  float b {0};
  bool s {false}, t {false}, u {false}, v {false}, x {false};
  std::string c,p,q,r,help;
  
  wex::cmdline cmdl(
     {{{"s1,s", "bool"}, [&](bool on){s = on;}},
      {{"s2,t", "bool"}, [&](bool on){t = on;}},
      {{"s3,u", "bool"}, [&](bool on){u = true;}},
      {{"s4,v", "bool"}, [&](bool on){v = on;}},
      {{"xx", "bool"}, [&](bool on){x = on;}}},
     {{{"o1,a", "int"}, {wex::cmdline::INT, [&](const std::any& i) {a = std::any_cast<int>(i);}}},
      {{"o2,b", "float"}, {wex::cmdline::FLOAT, [&](const std::any& f) {b = std::any_cast<float>(f);}}},
      {{"o3,c", "string"}, {wex::cmdline::STRING, [&](const std::any& s) {c = std::any_cast<std::string>(s);}}}},
     {{"rest", "rest"}, [&](const std::vector<std::string> & v) {
        p = v[0];
        q = v[1];
        r = v[2];}});

  const bool res(cmdl.parse(
    "-a 10 -b 5.1 -c test -s -t -u -v --xx one two three", help));
  
  CAPTURE( help );

  REQUIRE( res );

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
