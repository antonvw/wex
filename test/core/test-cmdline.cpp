////////////////////////////////////////////////////////////////////////////////
// Name:      test-cmdline.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/cmdline.h>
#include <wex/core/log-none.h>
#include <wex/test/test.h>

TEST_CASE("wex::cmdline")
{
  SECTION("default constructor")
  {
    wex::cmdline cmdl;

    SECTION("1")
    {
      wex::data::cmdline data("");
      REQUIRE(cmdl.parse(data));
    }

    SECTION("2")
    {
      wex::log_none      off;
      wex::data::cmdline data("xxx");
      REQUIRE(!cmdl.parse(data));
    }

    SECTION("3")
    {
      wex::data::cmdline data("-h");
      REQUIRE(!cmdl.parse(data));
    }
  }

  SECTION("constructor-no-standard-options")
  {
    int         a{0};
    float       b{0};
    bool        s{false}, t{false}, u{false}, w{false}, x{false};
    std::string c, p, q, r;

    wex::cmdline cmdl(
      {{{"s1,s", "bool"},
        [&](bool on)
        {
          s = on;
        }},
       {{"s2,t", "bool"},
        [&](bool on)
        {
          t = on;
        }},
       {{"s3,u", "bool"},
        [&](bool on)
        {
          u = true;
        }},
       {{"s4,w", "bool"},
        [&](bool on)
        {
          w = on;
        }},
       {{"xx", "bool"},
        [&](bool on)
        {
          x = on;
        }}},
      {{{"o1,a", "int"},
        {wex::cmdline::INT,
         [&](const std::any& i)
         {
           a = std::any_cast<int>(i);
         }}},
       {{"o2,b", "float"},
        {wex::cmdline::FLOAT,
         [&](const std::any& f)
         {
           b = std::any_cast<float>(f);
         }}},
       {{"o3,c", "string"},
        {wex::cmdline::STRING,
         [&](const std::any& s)
         {
           c = std::any_cast<std::string>(s);
         }}}},
      {{"rest", "rest"},
       [&](const std::vector<std::string>& v)
       {
         p = v[0];
         q = v[1];
         r = v[2];
       }},
      false);

    SECTION("help")
    {
      wex::data::cmdline data("-h");
      const bool         res(cmdl.parse(data));

      REQUIRE(!res);
      REQUIRE(!data.help().empty());
    }

    SECTION("parse")
    {
      wex::data::cmdline data(
        "-a 10 -b 5.1 -c test -s -t -u -w --xx one two three");
      const bool res(cmdl.parse(data));

      CAPTURE(data.help());

      REQUIRE(res);

      REQUIRE(a == 10);
      REQUIRE(b == 5.1f);
      REQUIRE(c == "test");
      REQUIRE(s);
      REQUIRE(t);
      REQUIRE(u);
      REQUIRE(w);
      REQUIRE(x);
      REQUIRE(p == "one");
      REQUIRE(q == "two");
      REQUIRE(r == "three");
    }

    SECTION("parse_set")
    {
      wex::log_none      off;
      wex::data::cmdline data("all");
      REQUIRE(cmdl.parse_set(data));
      REQUIRE(!data.help().empty());

      const bool res1(cmdl.parse_set(data.string("s1 s2")));

      CAPTURE(data.help());
      REQUIRE(res1);

      REQUIRE(!cmdl.parse_set(data.string("s9")));
      REQUIRE(cmdl.parse_set(data.string("s1?")));
      REQUIRE(cmdl.parse_set(data.string("nos1")));
    }
  }

  SECTION("constructor-standard-options")
  {
    bool        s{false};
    std::string a;

    wex::cmdline cmdl(
      {{{"s1,s", "bool"},
        [&](bool on)
        {
          s = on;
        }}},
      {{{"o1,a", "int"},
        {wex::cmdline::INT,
         [&](const std::any& i)
         {
           a = std::any_cast<int>(i);
         }}}});

    wex::data::cmdline data("-a 10 -s");
    REQUIRE(cmdl.parse(data));

    REQUIRE(wex::cmdline::get_output().empty());
    REQUIRE(wex::cmdline::get_scriptout().empty());
    REQUIRE(!wex::cmdline::is_echo());
    REQUIRE(!wex::cmdline::is_output());
    REQUIRE(!wex::cmdline::use_events());

    wex::data::cmdline data2("-a 10 -e -s -w www -x -X xxx -q 100");
    REQUIRE(cmdl.parse(data2));

    REQUIRE(wex::cmdline::get_output() == "xxx");
    REQUIRE(wex::cmdline::get_scriptout() == "www");
    REQUIRE(wex::cmdline::is_echo());
    REQUIRE(wex::cmdline::is_output());
    REQUIRE(wex::cmdline::use_events());
  }
}
