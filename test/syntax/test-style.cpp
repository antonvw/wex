////////////////////////////////////////////////////////////////////////////////
// Name:      test-style.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/log-none.h>
#include <wex/syntax/lexers.h>
#include <wex/syntax/style.h>
#include <wex/test/test.h>
#include <wx/stc/stc.h>

#include <numeric>

#include "test.h"

TEST_CASE("wex::style")
{
  SUBCASE("constructor")
  {
    REQUIRE(!wex::style().is_ok());
  }

  SUBCASE("constructor-no-value")
  {
    wex::log_none off;
    for (const auto& style : std::vector<std::pair<
           std::pair<std::string, std::set<int>>,
           std::pair<std::string, std::string>>>{
           {{"MARK_CIRCLE", {}}, {"ugly", "global"}},
           {{"mark_circle", {0}}, {"ugly", "global"}},
           {{"512", {}}, {"ugly", "global"}},
           {{"number,string,comment", {1, 4, 6}}, {"fore:blue", "cpp"}},
           {{"number,string,xxx", {4, 6}}, {"fore:black", "cpp"}},
           {{"xxx", {}}, {"fore:black", "cpp"}}})
    {
      // no, value, macro
      const wex::style test(
        style.first.first,
        style.second.first,
        style.second.second);

      if (!style.first.second.empty())
      {
        CAPTURE(style.first.first);
        REQUIRE(test.is_ok());
        REQUIRE(test.number() == *style.first.second.begin());
        REQUIRE(test.numbers() == style.first.second);
        REQUIRE(test.value() == style.second.first);
      }
      else
      {
        REQUIRE(!test.is_ok());
      }
    }
  }

  SUBCASE("constructor-xml")
  {
    pugi::xml_document doc;
    REQUIRE(doc.load_string("<style no = \"2\">string</style>"));

    REQUIRE(wex::style(doc.document_element(), "").number() == 2);
    REQUIRE(wex::style(doc.document_element(), "").value() == "fore:#ffab8f");
    REQUIRE(wex::style(doc.document_element(), "cpp").number() == 2);
    REQUIRE(wex::style(doc.document_element(), "").value() == "fore:#ffab8f");

    REQUIRE(doc.load_string("<style no = \"2\">styledefault+comment</style>"));

    REQUIRE(
      wex::style(doc.document_element(), "cpp").value().find("default") ==
      std::string::npos);
    REQUIRE(
      wex::style(doc.document_element(), "cpp").value().find("comment") ==
      std::string::npos);
    REQUIRE(
      wex::style(doc.document_element(), "cpp").value().find("+") ==
      std::string::npos);
  }

  SUBCASE("apply")
  {
    auto* stc = new wex::test::stc();

    SUBCASE("colour")
    {
      wex::log_none off;
      wex::style    style("32", "fore:gray 2,back:ivory 99");
      REQUIRE(!style.apply(stc));
      REQUIRE(style.is_ok());
      REQUIRE(style.contains_default_style());

      wex::lexers::get()->apply_default_style(
        [=](const std::string& back)
        {
          REQUIRE(wxColour(back).IsOk());
        },
        [=](const std::string& fore)
        {
          REQUIRE(wxColour(fore).IsOk());
        });
    }

    SUBCASE("empty")
    {
      wxStyledTextCtrl s;
      REQUIRE(!wex::style().apply(&s));
    }

    SUBCASE("mark_circle")
    {
      wex::style       style("mark_circle", "0");
      wxStyledTextCtrl s;

      REQUIRE(!style.apply(&s));
      REQUIRE(style.apply(stc));
      REQUIRE(style.is_ok());
      REQUIRE(!style.contains_default_style());
    }
  }
}
