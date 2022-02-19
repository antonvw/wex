////////////////////////////////////////////////////////////////////////////////
// Name:      test-core.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "../test.h"
#include <wex/core/core.h>

#include <vector>

TEST_CASE("wex::core")
{
  std::vector<int> cs{'(', ')', '{', '<', '>'};

  SUBCASE("clipboard")
  {
    REQUIRE(wex::clipboard_add("test"));
    REQUIRE(wex::clipboard_get() == "test");
  }

  SUBCASE("ellipsed")
  {
    REQUIRE(wex::ellipsed("xxx").find("...") != std::string::npos);
  }

  SUBCASE("find_after")
  {
    REQUIRE(wex::rfind_after("nospace", " ") == "nospace");
    REQUIRE(wex::find_after("nospace", " ") == "nospace");
    REQUIRE(wex::rfind_after("some space and more", " ") == "more");
    REQUIRE(wex::find_after("some space and more", " ") == "space and more");
    REQUIRE(wex::rfind_after("some space and more", "m") == "ore");
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

  SUBCASE("get_find_result")
  {
    REQUIRE(
      wex::get_find_result("test", true, true).find("test") !=
      std::string::npos);
    REQUIRE(
      wex::get_find_result("test", true, false).find("test") !=
      std::string::npos);
    REQUIRE(
      wex::get_find_result("test", false, true).find("test") !=
      std::string::npos);
    REQUIRE(
      wex::get_find_result("test", false, false).find("test") !=
      std::string::npos);

    REQUIRE(
      wex::get_find_result("%d", true, true).find("%d") != std::string::npos);
    REQUIRE(
      wex::get_find_result("%d", true, false).find("%d") != std::string::npos);
    REQUIRE(
      wex::get_find_result("%d", false, true).find("%d") != std::string::npos);
    REQUIRE(
      wex::get_find_result("%d", false, false).find("%d") != std::string::npos);
  }

  SUBCASE("get_iconid")
  {
    REQUIRE(wex::get_iconid(wex::test::get_path("test.h")) != -1);
  }

  SUBCASE("get_number_of_lines")
  {
    REQUIRE(wex::get_number_of_lines("test") == 1);
    REQUIRE(wex::get_number_of_lines("test\n") == 2);
    REQUIRE(wex::get_number_of_lines("test\ntest") == 2);
    REQUIRE(wex::get_number_of_lines("test\ntest\n") == 3);
    REQUIRE(wex::get_number_of_lines("test\rtest\r") == 3);
    REQUIRE(wex::get_number_of_lines("test\r\ntest\n") == 3);

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
    REQUIRE(wex::get_word(word) == "is");
    REQUIRE(wex::get_word(word) == "a");
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
    REQUIRE(wex::quoted_find("te st") == "\" te st\"");
  }

  SUBCASE("translate")
  {
    REQUIRE(
      wex::translate("hello @PAGENUM@ from @PAGESCNT@", 1, 2).find("@") ==
      std::string::npos);
  }
}
