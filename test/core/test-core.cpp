////////////////////////////////////////////////////////////////////////////////
// Name:      test-core.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/core.h>
#include <wex/core/types.h>
#include <wex/test/test.h>

#include <vector>

TEST_CASE("wex::core")
{
  wex::ints_t cs{'(', ')', '{', '<', '>'};

  SUBCASE("clipboard")
  {
    REQUIRE(wex::clipboard_add("test"));
    REQUIRE(wex::clipboard_get() == "test");
  }

  SUBCASE("ellipsed")
  {
    REQUIRE(wex::ellipsed("xxx").contains("..."));
  }

  SUBCASE("find_after")
  {
    REQUIRE(wex::rfind_after("nospace", " ") == "nospace");
    REQUIRE(wex::find_after("nospace", " ") == "nospace");
    REQUIRE(wex::rfind_after("some space and more", " ") == "more");
    REQUIRE(wex::find_after("some space and more", " ") == "space and more");
    REQUIRE(wex::find_after("some space and more", "space") == " and more");
    REQUIRE(wex::rfind_after("some space and more", "m") == "ore");
    REQUIRE(wex::rfind_after("some space and more", " m") == "ore");
  }

  SUBCASE("find_before")
  {
    REQUIRE(wex::rfind_before("nospace", " ") == "nospace");
    REQUIRE(wex::find_before("nospace", " ") == "nospace");
    REQUIRE(wex::rfind_before("some space and more", " ") == "some space and");
    REQUIRE(wex::find_before("some space and more", " ") == "some");
    REQUIRE(wex::rfind_before("some space and more", "m") == "some space and ");
  }

  SUBCASE("find_tail")
  {
    REQUIRE(wex::find_tail("test") == std::string("test"));
    REQUIRE(wex::find_tail("test", 3) == std::string("est"));
    REQUIRE(wex::find_tail("testtest", 3) == std::string("est"));
    REQUIRE(wex::find_tail("testtest", 6) == std::string("...est"));
    REQUIRE(wex::find_tail("testtest", 9) == std::string("testtest"));
  }

  SUBCASE("get_number_of_lines")
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

  SUBCASE("get_string_set")
  {
    REQUIRE(wex::get_string_set({"one", "two", "three"}) == "one three two ");
    REQUIRE(wex::get_string_set({"one", "two", "three"}, 4) == "three ");
  }

  SUBCASE("get_word")
  {
    std::string word("this is a test");
    REQUIRE(wex::get_word(word) == "this");
    REQUIRE(word == "is a test");
    REQUIRE(wex::get_word(word) == "is");
    REQUIRE(wex::get_word(word) == "a");

    std::string other("    code improvements");
    REQUIRE(wex::get_word(other) == "code");
    REQUIRE(other == "improvements");

    std::string spaces("    ");
    REQUIRE(wex::get_word(spaces).empty());
    REQUIRE(spaces.empty());

    std::string with_nl("test\ntest2");
    REQUIRE(wex::get_word(with_nl) == "test");
    REQUIRE(wex::get_word(with_nl) == "test2");
  }

  SUBCASE("icompare")
  {
    REQUIRE(wex::icompare("", "") == 0);
    REQUIRE(wex::icompare("test", "test") == 0);
    REQUIRE(wex::icompare("test", "tESt") == 0);

    REQUIRE(wex::icompare("", "x") != 0);
    REQUIRE(wex::icompare("test", "xtESt") != 0);
    REQUIRE(wex::icompare("test", "tEStx") != 0);
  }

  SUBCASE("icontains")
  {
    REQUIRE(wex::icontains("test", ""));
    REQUIRE(wex::icontains("test", "e"));
    REQUIRE(wex::icontains("test", "E"));
    REQUIRE(wex::icontains("test", "TEST"));

    REQUIRE(!wex::icontains("e", "test"));
    REQUIRE(!wex::icontains("test", "f"));
    REQUIRE(!wex::icontains("test", "TESTx"));
  }

  SUBCASE("is_brace")
  {
    for (const auto& c : cs)
    {
      REQUIRE(wex::is_brace(c));
    }

    REQUIRE(!wex::is_brace('a'));
  }

  SUBCASE("is_codeword_separator")
  {
    cs.insert(cs.end(), {',', ';', ':', '@'});

    for (const auto& c : cs)
    {
      REQUIRE(wex::is_codeword_separator(c));
    }

    REQUIRE(!wex::is_codeword_separator('x'));
  }

  SUBCASE("matches_one_of")
  {
    REQUIRE(!wex::matches_one_of("test.txt", "*.cpp"));
    REQUIRE(wex::matches_one_of("test.txt", "*.txt"));
    REQUIRE(wex::matches_one_of("test.txt", "*.cpp;*.txt"));
  }

  SUBCASE("quoted")
  {
    REQUIRE(wex::quoted("test") == "'test'");
    REQUIRE(wex::quoted("%d") == "'%d'");
  }

  SUBCASE("quoted_find")
  {
    REQUIRE(wex::quoted_find("test") == "test");
    REQUIRE(wex::quoted_find("te st") == "\"te st\"");
  }
}
