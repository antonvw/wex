////////////////////////////////////////////////////////////////////////////////
// Name:      test-blame.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019-2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "../test.h"
#include <wex/core/config.h>
#include <wex/factory/blame.h>

TEST_CASE("wex::blame")
{
  SUBCASE("default constructor and get")
  {
    REQUIRE(!wex::blame().use());
    REQUIRE(!std::get<0>(wex::blame().get("")));
  }

  SUBCASE("constructor xml node and get")
  {
    pugi::xml_document doc;

    REQUIRE(doc.load_string(
      "<vcs name=\"git\" "
      "blame-format=\"(^[a-zA-Z0-9]+) .*\\(([a-zA-Z "
      "]+)\\s+([0-9]{2,4}.[0-9]{2}.[0-9]{2}.[0-9:]{8}) .[0-9]+\\s+([0-9]+)\" "
      "date-format=\"%Y-%m-%d %H:%M:%S\" "
      "date-print=\"10\" >"
      "</vcs>"));

    wex::blame blame(doc.document_element());

    REQUIRE(blame.use());

    const std::string text(
      ""
      "bf5d87cc src/http_travel.cpp (A unknown user 2019-02-01 12:20:06 +0100 "
      "15) const std::string& http_travel:get_country()");

    wex::config("blame", "author").set(true);
    REQUIRE(wex::config("blame.author").get(false));

    wex::config("blame", "date").set(true);
    REQUIRE(wex::config("blame.author").get(false));
    REQUIRE(wex::config("blame.date").get(false));

    wex::config("blame", "id").set(true);
    REQUIRE(wex::config("blame.author").get(false));
    REQUIRE(wex::config("blame.date").get(false));
    REQUIRE(wex::config("blame.id").get(false));

    REQUIRE(!std::get<0>(blame.get("")));
    REQUIRE(!std::get<0>(blame.get(std::string())));
    REQUIRE(std::get<0>(blame.get(text)));
    REQUIRE(
      std::get<1>(blame.get(text)).find("A unknown user") != std::string::npos);
    REQUIRE(
      std::get<1>(blame.get(text)).find("2019-02-01") != std::string::npos);
    REQUIRE(std::get<1>(blame.get(text)).find("bf5d87cc") != std::string::npos);
    REQUIRE(
      std::get<2>(blame.get(text)) != wex::lexers::margin_style_t::UNKNOWN);
    REQUIRE(std::get<3>(blame.get(text)) == 14);

    wex::config("blame", "author").set(false);
    REQUIRE(
      std::get<1>(blame.get(text)).find("A unknown user") == std::string::npos);
  }
}
