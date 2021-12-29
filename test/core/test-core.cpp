////////////////////////////////////////////////////////////////////////////////
// Name:      test-core.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "../test.h"
#include <wex/core/core.h>

#include <vector>

TEST_CASE("wex::core")
{
  std::vector<int> cs{'(', ')', '{', '<', '>'};

  SUBCASE("after")
  {
    REQUIRE(wex::after("nospace", ' ', false) == "nospace");
    REQUIRE(wex::after("nospace", ' ', true) == "nospace");
    REQUIRE(wex::after("some space and more", ' ', false) == "more");
    REQUIRE(wex::after("some space and more", ' ', true) == "space and more");
    REQUIRE(wex::after("some space and more", 'm', false) == "ore");
  }

  SUBCASE("before")
  {
    REQUIRE(wex::before("nospace", ' ', false) == "nospace");
    REQUIRE(wex::before("nospace", ' ', true) == "nospace");
    REQUIRE(wex::before("some space and more", ' ', false) == "some space and");
    REQUIRE(wex::before("some space and more", ' ', true) == "some");
    REQUIRE(
      wex::before("some space and more", 'm', false) == "some space and ");
  }

  SUBCASE("clipboard")
  {
    REQUIRE(wex::clipboard_add("test"));
    REQUIRE(wex::clipboard_get() == "test");
  }

  SUBCASE("ellipsed")
  {
    REQUIRE(wex::ellipsed("xxx").find("...") != std::string::npos);
  }

  SUBCASE("first_of")
  {
    REQUIRE(wex::first_of("this is ok", "x") == std::string());
    REQUIRE(wex::first_of("this is ok", " ;,") == "is ok");
    REQUIRE(wex::first_of("this is ok", " ;,i") == "s is ok");
    REQUIRE(
      wex::first_of(
        "this is ok",
        " ;,i",
        std::string::npos,
        wex::first_of_t().set(wex::FIRST_OF_FROM_END)) == "ok");
    REQUIRE(
      wex::first_of(
        "this is ok",
        " ",
        0,
        wex::first_of_t().set(wex::FIRST_OF_BEFORE)) == "this");
    REQUIRE(
      wex::first_of(
        "this is ok",
        "x",
        0,
        wex::first_of_t().set(wex::FIRST_OF_BEFORE)) == "this is ok");
  }

  SUBCASE("get_endoftext")
  {
    REQUIRE(wex::get_endoftext("test", 3).size() == 3);
    REQUIRE(wex::get_endoftext("testtest", 3).size() == 3);
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

  SUBCASE("translate")
  {
    REQUIRE(
      wex::translate("hello @PAGENUM@ from @PAGESCNT@", 1, 2).find("@") ==
      std::string::npos);
  }
}
