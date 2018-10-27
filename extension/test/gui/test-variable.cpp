////////////////////////////////////////////////////////////////////////////////
// Name:      test-variable.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <tuple>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/variable.h>
#include <wex/managedframe.h>
#include <wex/stc.h>
#include "test.h"

TEST_CASE("wex::variable")
{
  wex::ex* ex = new wex::ex(GetSTC());

  SUBCASE("Default constructor")
  {
    std::string value;
    REQUIRE( wex::variable("test").GetName() == "test");
    REQUIRE( wex::variable("test").GetValue().empty());
    REQUIRE(!wex::variable("test").IsBuiltIn());
    REQUIRE(!wex::variable("test").IsTemplate());

    wex::variable var("test");
    var.SetAskForInput(false);

    REQUIRE( var.Expand(nullptr));
    REQUIRE( var.Expand(ex));
    REQUIRE( var.Expand(value));
  }

  pugi::xml_document doc;

  SUBCASE("XML")
  {
    for (const auto& it : std::vector<std::tuple<
      std::string, std::string, std::string, std::string>> {
      {"Created", "BUILTIN", "", ""},     
#ifdef __UNIX__
      {"HOME", "ENVIRONMENT", "", ""}, 
#endif
      {"aa", "OTHER", "", ""},
      {"template", "TEMPLATE", "xxx.txt", "xxx.txt"},
      {"cc", "INPUT", "one", "one"},       
      {"dd", "INPUT-ONCE", "@Year@", "2018"},
      {"ee", "INPUT-SAVE", "three", "three"}})
    {
      const std::string text(
        "<variable name=\"" + std::get<0>(it) +
        "\" type=\"" + std::get<1>(it) +
        "\">" + std::get<2>(it) +
        "</variable>");
      CAPTURE( text );
      REQUIRE( doc.load_string(text.c_str()));
      pugi::xml_node node = doc.document_element();

      wex::variable var(node);
      var.SetAskForInput(false);

      REQUIRE( var.GetName() == std::get<0>(it));
      REQUIRE( var.GetValue() == std::get<2>(it));
      if (var.GetName() == "template")
        REQUIRE(!var.Expand(ex));
      else
        REQUIRE( var.Expand(ex));
      REQUIRE( var.GetValue() == std::get<3>(it));

      var.Save(doc);

      node.remove_attribute("name");
    }
  }
  
  SUBCASE("builtin")
  {
    for (const auto& it : GetBuiltinVariables())
    {
      const std::string text(
        "<variable name =\"" + it + 
        "\" type=\"BUILTIN\"></variable>");
      CAPTURE( text );
      REQUIRE( doc.load_string(text.c_str()));
      pugi::xml_node node = doc.document_element();

      wex::variable var(node);

      REQUIRE( var.GetName() == it);
      REQUIRE( var.GetValue().empty());
      REQUIRE( var.IsBuiltIn());
      REQUIRE( var.Expand(ex));

      std::string content;
      REQUIRE( var.Expand(content, ex));

      if (it == "Year")
      {
        REQUIRE( content.find("20") == 0); // start of year
      }
      
      node.remove_attribute("name");
    }
  }
}
