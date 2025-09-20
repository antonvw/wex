////////////////////////////////////////////////////////////////////////////////
// Name:      test-blame.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "../factory/test.h"
#include <wex/core/config.h>
#include <wex/syntax/blame.h>

TEST_CASE("wex::blame")
{
  SECTION("default-constructor")
  {
    REQUIRE(!wex::blame().use());
    REQUIRE(!wex::blame().use());
    REQUIRE(wex::blame().caption().empty());
    REQUIRE(!wex::blame().parse(wex::path(), ""));
    REQUIRE(wex::blame().info().empty());
    REQUIRE(wex::blame().style() == wex::blame::margin_style_t::UNKNOWN);
    REQUIRE(wex::blame().vcs_name().empty());
  }

  SECTION("constructor-xml")
  {
    pugi::xml_document doc;

    // clang-format off
    REQUIRE(doc.load_string(
      "<vcs name=\"git\" "
      "blame-format=\"(^[a-zA-Z0-9]+) (.*)\\(([a-zA-Z "
      "]+)\\s+([0-9]{2,4}.[0-9]{2}.[0-9]{2}.[0-9:]{8}) .[0-9]+\\s+([0-9]+)\\) (.*)\" "
      "date-format=\"%Y-%m-%d %H:%M:%S\" "
      "date-print=\"10\" >"
      "</vcs>"));
    // clang-format on

    wex::blame blame(doc.document_element());

    REQUIRE(blame.use());
    REQUIRE(blame.vcs_name() == "git");

    const std::string text(
      ""
      "bf5d87cc src/http_travel.cpp (A unknown user 2019-02-01 12:20:06 +0100 "
      "15) const std::string& http_travel:get_country");

    wex::config("blame", "author").set(true);
    REQUIRE(wex::config("blame.author").get(false));

    wex::config("blame", "date").set(true);
    REQUIRE(wex::config("blame.author").get(false));
    REQUIRE(wex::config("blame.date").get(false));

    wex::config("blame", "id").set(true);
    REQUIRE(wex::config("blame.author").get(false));
    REQUIRE(wex::config("blame.date").get(false));
    REQUIRE(wex::config("blame.id").get(false));

    REQUIRE(blame.parse(wex::path(), text));

    REQUIRE(blame.info().contains("A unknown user"));
    REQUIRE(blame.info().contains("2019-02-01"));
    REQUIRE(blame.info().contains("bf5d87cc"));

    REQUIRE(blame.style() != wex::blame::margin_style_t::UNKNOWN);

    REQUIRE(blame.line_no() == 14);
    REQUIRE(blame.line_text().contains("get_country"));

    wex::config("blame", "author").set(false);
    REQUIRE(blame.parse(wex::path(), text));
    REQUIRE(!blame.info().contains("A unknown user"));
  }

  SECTION("set")
  {
    wex::blame blame;
    blame.caption("hello world");
    REQUIRE(blame.caption() == "hello world");
  }

  SECTION("skip_info")
  {
    wex::blame blame;
    REQUIRE(!blame.skip_info());

    blame.skip_info(true);
    REQUIRE(blame.skip_info());
  }

  SECTION("static")
  {
    auto* stc = new wex::test::stc();
    stc->set_text("more text\notherline\nother line");

    REQUIRE(wex::blame::margin_renamed(stc).empty());
  }
}
