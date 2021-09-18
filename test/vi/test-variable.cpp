////////////////////////////////////////////////////////////////////////////////
// Name:      test-variable.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/ex.h>
#include <wex/variable.h>

import<tuple>;

#include "test.h"

TEST_CASE("wex::variable")
{
  auto* ex = new wex::ex(get_stc());

  SUBCASE("constructor")
  {
    std::string value;
    REQUIRE(wex::variable("test").get_name() == "test");
    REQUIRE(wex::variable("test").get_value().empty());
    REQUIRE(!wex::variable("test").is_builtin());
    REQUIRE(!wex::variable("test").is_template());

    wex::variable var("test");
    var.set_ask_for_input(false);

    REQUIRE(var.expand(nullptr));
    REQUIRE(var.expand(ex));
    REQUIRE(var.expand(value));
  }

  pugi::xml_document doc;

  SUBCASE("XML")
  {
    for (const auto& it : std::vector<
           std::tuple<std::string, std::string, std::string, std::string>>{
           {"Created", "BUILTIN", "", ""},
#ifdef __UNIX__
           {"HOME", "ENVIRONMENT", "", ""},
           {"process", "PROCESS", "echo hoi", "echo hoi"},
#endif
           {"aa", "OTHER", "", ""},
           {"template", "TEMPLATE", "xxx.txt", "xxx.txt"},
           {"fix", "FIXED", "constant value", "constant value"},
           {"cc", "INPUT", "one", "one"},
           {"ee", "INPUT-SAVE", "three", "three"}})
    {
      const std::string text(
        "<variable name=\"" + std::get<0>(it) + "\" type=\"" + std::get<1>(it) +
        "\">" + std::get<2>(it) + "</variable>");

      CAPTURE(text);
      REQUIRE(doc.load_string(text.c_str()));

      auto node(doc.document_element());

      wex::variable var(node);
      var.set_ask_for_input(false);

      REQUIRE(var.get_name() == std::get<0>(it));

      if (var.get_name() != "process")
      {
        REQUIRE(var.get_value() == std::get<2>(it));
      }

      if (var.get_name() == "template")
        REQUIRE(!var.expand(ex));
      else
        REQUIRE(var.expand(ex));

      REQUIRE(var.get_value() == std::get<3>(it));

      var.save(doc);

      node.remove_attribute("name");
    }
  }

  SUBCASE("builtin")
  {
    for (const auto& it : get_builtin_variables())
    {
      const std::string text(
        "<variable name =\"" + it + "\" type=\"BUILTIN\"></variable>");

      CAPTURE(text);
      REQUIRE(doc.load_string(text.c_str()));

      auto node(doc.document_element());

      wex::variable var(node);

      REQUIRE(var.get_name() == it);
      REQUIRE(var.get_value().empty());
      REQUIRE(var.is_builtin());
      REQUIRE(var.expand(ex));

      std::string content;
      REQUIRE(var.expand(content, ex));

      if (it == "Year")
      {
        REQUIRE(content.starts_with("20")); // start of year
      }

      node.remove_attribute("name");
    }
  }

  SUBCASE("format")
  {
    for (const auto& it : std::vector<
           std::tuple<std::string, std::string, std::string, std::string>>{
           {"Year", "BUILTIN", "%Y", "2021"},
           {"Date", "BUILTIN", "%Y", "2021"}})
    {
      const std::string text(
        "<variable name=\"" + std::get<0>(it) + "\" type=\"" + std::get<1>(it) +
        "\" format=\"" + std::get<2>(it) + "\">" + "</variable>");

      CAPTURE(text);
      REQUIRE(doc.load_string(text.c_str()));

      const auto node(doc.document_element());

      wex::variable var(node);

      REQUIRE(var.get_name() == std::get<0>(it));
      REQUIRE(var.get_value().empty());
      REQUIRE(var.is_builtin());

      std::string content;
      REQUIRE(var.expand(content, ex));
      REQUIRE(content == std::get<3>(it));
    }
  }
}
