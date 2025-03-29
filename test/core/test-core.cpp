////////////////////////////////////////////////////////////////////////////////
// Name:      test-core.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/core.h>
#include <wex/core/types.h>
#include <wex/test/test.h>

#include <vector>

TEST_CASE("wex::core")
{
  wex::ints_t cs{'(', ')', '{', '<', '>'};

  SECTION("clipboard")
  {
    REQUIRE(wex::clipboard_add("test"));
    REQUIRE(wex::clipboard_get() == "test");
  }

  SECTION("ellipsed")
  {
    REQUIRE(wex::ellipsed("xxx").contains("..."));
  }

  SECTION("find_after")
  {
    REQUIRE(wex::rfind_after("nospace", " ") == "nospace");
    REQUIRE(wex::find_after("nospace", " ") == "nospace");
    REQUIRE(wex::rfind_after("some space and more", " ") == "more");
    REQUIRE(wex::find_after("some space and more", " ") == "space and more");
    REQUIRE(wex::find_after("some space and more", "space") == " and more");
    REQUIRE(wex::rfind_after("some space and more", "m") == "ore");
    REQUIRE(wex::rfind_after("some space and more", " m") == "ore");
  }

  SECTION("find_before")
  {
    REQUIRE(wex::rfind_before("nospace", " ") == "nospace");
    REQUIRE(wex::find_before("nospace", " ") == "nospace");
    REQUIRE(wex::rfind_before("some space and more", " ") == "some space and");
    REQUIRE(wex::find_before("some space and more", " ") == "some");
    REQUIRE(wex::rfind_before("some space and more", "m") == "some space and ");
  }

  SECTION("find_tail")
  {
    REQUIRE(wex::find_tail("test") == std::string("test"));
    REQUIRE(wex::find_tail("test", 3) == std::string("est"));
    REQUIRE(wex::find_tail("testtest", 3) == std::string("est"));
    REQUIRE(wex::find_tail("testtest", 6) == std::string("...est"));
    REQUIRE(wex::find_tail("testtest", 9) == std::string("testtest"));
  }

  SECTION("get_number_of_lines")
  {
    REQUIRE(wex::get_number_of_lines("test") == 1);
    REQUIRE(wex::get_number_of_lines("test\n") == 2);
    REQUIRE(wex::get_number_of_lines("test\ntest") == 2);
    REQUIRE(wex::get_number_of_lines("test\ntest\n") == 3);
    REQUIRE(wex::get_number_of_lines("test\rtest\r") == 3);
    REQUIRE(wex::get_number_of_lines("test\r\ntest\n") == 3);

    // A DOS file.
    REQUIRE(wex::get_number_of_lines("test\rtest\n\n\r\r\r\n\n\n") == 6);

    // No DOS file.
    REQUIRE(wex::get_number_of_lines("test\rtest\n\n\r\r\rx\n\n\n") == 10);

    // trimmed
    REQUIRE(wex::get_number_of_lines("test\r\ntest\n\n\n", true) == 2);
    REQUIRE(wex::get_number_of_lines("test\r\ntest\n\n", true) == 2);
    REQUIRE(wex::get_number_of_lines("test\r\ntest\n\n", true) == 2);
  }

  SECTION("get_string_set")
  {
    REQUIRE(wex::get_string_set({"one", "two", "three"}) == "one three two ");
    REQUIRE(wex::get_string_set({"one", "two", "three"}, 4) == "three ");
  }

  SECTION("icompare")
  {
    REQUIRE(wex::icompare("", "") == 0);
    REQUIRE(wex::icompare("test", "test") == 0);
    REQUIRE(wex::icompare("test", "tESt") == 0);

    REQUIRE(wex::icompare("", "x") != 0);
    REQUIRE(wex::icompare("test", "xtESt") != 0);
    REQUIRE(wex::icompare("test", "tEStx") != 0);
  }

  SECTION("icontains")
  {
    REQUIRE(wex::icontains("test", ""));
    REQUIRE(wex::icontains("test", "e"));
    REQUIRE(wex::icontains("test", "E"));
    REQUIRE(wex::icontains("test", "TEST"));

    REQUIRE(!wex::icontains("e", "test"));
    REQUIRE(!wex::icontains("test", "f"));
    REQUIRE(!wex::icontains("test", "TESTx"));
  }

  SECTION("is_brace")
  {
    for (const auto& c : cs)
    {
      REQUIRE(wex::is_brace(c));
    }

    REQUIRE(!wex::is_brace('a'));
  }

  SECTION("is_codeword_separator")
  {
    cs.insert(cs.end(), {',', ';', ':', '@'});

    for (const auto& c : cs)
    {
      REQUIRE(wex::is_codeword_separator(c));
    }

    REQUIRE(!wex::is_codeword_separator('x'));
  }

  SECTION("matches_one_of")
  {
    REQUIRE(!wex::matches_one_of("test.txt", "*.cpp"));
    REQUIRE(wex::matches_one_of("test.txt", "*.txt"));
    REQUIRE(wex::matches_one_of("test.txt", "*.cpp;*.txt"));

    REQUIRE(!wex::matches_one_of("test.txt", "*.txt", true));
    REQUIRE(wex::matches_one_of("test.txt", ".*.txt", true));
    REQUIRE(wex::matches_one_of("test.txt", "est.txt", true));
  }

  SECTION("quoted")
  {
    REQUIRE(wex::quoted("test") == "'test'");
    REQUIRE(wex::quoted("test", '%') == "%test%");
    REQUIRE(wex::quoted("%d") == "'%d'");
  }

  SECTION("quoted_find")
  {
    REQUIRE(wex::quoted_find("test") == "test");
    REQUIRE(wex::quoted_find("te st") == "\"te st\"");
  }
}
