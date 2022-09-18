////////////////////////////////////////////////////////////////////////////////
// Name:      test-blame.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "test.h"
#include <wex/core/config.h>
#include <wex/factory/blame.h>

TEST_CASE("wex::blame")
{
  SUBCASE("default-constructor")
  {
    REQUIRE(!wex::blame().use());
    REQUIRE(!wex::blame().use());
    REQUIRE(wex::blame().caption().empty());
    REQUIRE(!wex::blame().parse(wex::path(), ""));
    REQUIRE(wex::blame().info().empty());
    REQUIRE(wex::blame().style() == wex::lexers::margin_style_t::UNKNOWN);
    REQUIRE(wex::blame().vcs_name().empty());
  }

  SUBCASE("constructor-xml")
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

    REQUIRE(blame.info().find("A unknown user") != std::string::npos);
    REQUIRE(blame.info().find("2019-02-01") != std::string::npos);
    REQUIRE(blame.info().find("bf5d87cc") != std::string::npos);

    REQUIRE(blame.style() != wex::lexers::margin_style_t::UNKNOWN);

    REQUIRE(blame.line_no() == 14);
    REQUIRE(blame.line_text().find("get_country") != std::string::npos);

    wex::config("blame", "author").set(false);
    REQUIRE(blame.parse(wex::path(), text));
    REQUIRE(blame.info().find("A unknown user") == std::string::npos);
  }

  SUBCASE("set")
  {
    wex::blame blame;
    blame.caption("hello world");
    REQUIRE(blame.caption() == "hello world");
  }

  SUBCASE("skip_info")
  {
    wex::blame blame;
    REQUIRE(!blame.skip_info());

    blame.skip_info(true);
    REQUIRE(blame.skip_info());
  }

  SUBCASE("static")
  {
    auto* stc = new wex::test::stc();
    stc->set_text("more text\notherline\nother line");

    REQUIRE(wex::blame::margin_renamed(stc).empty());
  }
}
