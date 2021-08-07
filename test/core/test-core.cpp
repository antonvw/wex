////////////////////////////////////////////////////////////////////////////////
// Name:      test-core.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <vector>

#include "../test.h"
#include <wex/core.h>
#include <wex/property.h>
#include <wex/style.h>

TEST_CASE("wex::core" * doctest::may_fail())
{
  std::vector<int> cs{'(', ')', '{', '<', '>'};

  const std::string rect("012z45678901234567890\n"
                         "123y56789012345678901\n"
                         "234x67890123456789012\n"
                         "345a78901234567890123\n"
                         "456b89012345678901234\n");

  const std::string sorted("012a78908901234567890\n"
                           "123b89019012345678901\n"
                           "234x67890123456789012\n"
                           "345y56781234567890123\n"
                           "456z45672345678901234\n");

  SUBCASE("after")
  {
    REQUIRE(wex::after("nospace", ' ', false) == "nospace");
    REQUIRE(wex::after("nospace", ' ', true) == "nospace");
    REQUIRE(wex::after("some space and more", ' ', false) == "more");
    REQUIRE(wex::after("some space and more", ' ', true) == "space and more");
    REQUIRE(wex::after("some space and more", 'm', false) == "ore");
  }

  SUBCASE("auto_complete_filename")
  {
    REQUIRE(std::get<0>(wex::auto_complete_filename("te")));
    REQUIRE(std::get<1>(wex::auto_complete_filename("te")) == "st");
    REQUIRE(!std::get<0>(wex::auto_complete_filename("XX")));

#ifdef __UNIX__
    REQUIRE(std::get<0>(wex::auto_complete_filename("/usr/local/l")));

    // we are in wex/test/data
    REQUIRE(std::get<0>(wex::auto_complete_filename("../../src/v")));
#endif
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

  SUBCASE("browser_search")
  {
    // Causes travis to hang.
    // REQUIRE( wex::browser_search("test"));
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

  SUBCASE("match")
  {
    std::vector<std::string> v;
    REQUIRE(wex::match("hllo", "hello world", v) == -1);
    REQUIRE(wex::match("hello", "hello world", v) == 0);
    REQUIRE(wex::match("([0-9]+)ok([0-9]+)nice", "19999ok245nice", v) == 2);
    REQUIRE(wex::match("(\\d+)ok(\\d+)nice", "19999ok245nice", v) == 2);
    REQUIRE(wex::match(" ([\\d\\w]+)", " 19999ok245nice ", v) == 1);
    REQUIRE(
      wex::match("([?/].*[?/])(,[?/].*[?/])([msy])", "/xx/,/yy/y", v) == 3);
  }

  SUBCASE("matches_one_of")
  {
    REQUIRE(!wex::matches_one_of("test.txt", "*.cpp"));
    REQUIRE(wex::matches_one_of("test.txt", "*.txt"));
    REQUIRE(wex::matches_one_of("test.txt", "*.cpp;*.txt"));
  }

  SUBCASE("node_properties")
  {
    std::vector<wex::property> properties;
    pugi::xml_document         doc;

    REQUIRE(doc.load_string("<properties>"
                            "  <property name = \"fold.comment\">2</property>"
                            "</properties>"));
    auto node = doc.document_element();

    wex::node_properties(&node, properties);

    REQUIRE(properties.size() == 1);
  }

  SUBCASE("node_styles")
  {
    std::vector<wex::style> styles;
    pugi::xml_document      doc;

    REQUIRE(doc.load_string("<styles>"
                            "  <style no = \"2\">string</style>"
                            "</styles>"));

    auto node = doc.document_element();

    wex::node_styles(&node, "cpp", styles);

    REQUIRE(styles.size() == 1);
  }

  SUBCASE("print_caption")
  {
    REQUIRE(
      wex::print_caption(wex::path("test")).find("test") != std::string::npos);
  }

  SUBCASE("print_footer")
  {
    REQUIRE(wex::print_footer().find("@") != std::string::npos);
  }

  SUBCASE("print_header")
  {
    REQUIRE(
      wex::print_header(wex::test::get_path("test.h")).find("test") !=
      std::string::npos);
  }

  SUBCASE("quoted")
  {
    REQUIRE(wex::quoted("test") == "'test'");
    REQUIRE(wex::quoted("%d") == "'%d'");
  }

  SUBCASE("sort")
  {
    REQUIRE(wex::sort("z\ny\nx\n", 0, 0, "\n") == "x\ny\nz\n");
    REQUIRE(
      wex::sort(
        "z\ny\nx\n",
        wex::string_sort_t().set(wex::STRING_SORT_DESCENDING),
        0,
        "\n") == "z\ny\nx\n");
    REQUIRE(wex::sort("z\nz\ny\nx\n", 0, 0, "\n") == "x\ny\nz\nz\n");
    REQUIRE(
      wex::sort(
        "z\nz\ny\nx\n",
        wex::string_sort_t().set(wex::STRING_SORT_UNIQUE),
        0,
        "\n") == "x\ny\nz\n");
    REQUIRE(wex::sort(rect, 0, 3, "\n", 5) == sorted);
  }

  SUBCASE("translate")
  {
    REQUIRE(
      wex::translate("hello @PAGENUM@ from @PAGESCNT@", 1, 2).find("@") ==
      std::string::npos);
  }
}
