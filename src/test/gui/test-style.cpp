////////////////////////////////////////////////////////////////////////////////
// Name:      test-style.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <numeric>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/style.h>
#include <wex/managedframe.h>
#include <wex/stc.h>
#include "test.h"

TEST_CASE("wex::style")
{
  SUBCASE("Default constructor")
  {
    REQUIRE(!wex::style().is_ok() );
  }
  
  SUBCASE("Constructor using no and value")
  {
    for (const auto& style : std::vector<
      std::pair<
        std::pair<std::string,std::string>,
        std::pair<std::string,std::string>>> {
      {{"MARK_CIRCLE",""}, {"ugly","global"}},
      {{"mark_circle","0 "}, {"ugly","global"}},
      {{"512",""}, {"ugly","global"}},
      {{"number,string,comment","1 4 6 "}, {"fore:blue", "cpp"}},
      {{"number,string,xxx","4 6 "}, {"fore:black", "cpp"}},
      {{"xxx",""}, {"fore:black", "cpp"}}})
    {
      // no, value, macro
      const wex::style test(
        style.first.first, style.second.first, style.second.second);
      
      if (!style.first.second.empty())
      {
        REQUIRE( test.is_ok());
        REQUIRE( test.number() == style.first.second);
        REQUIRE( test.value() == style.second.first);
      }
      else
      {
        REQUIRE(!test.is_ok());
      }
    }
  }

  SUBCASE("Constructor using xml node")
  {
    pugi::xml_document doc;
    REQUIRE( doc.load_string("<style no = \"2\">string</style>"));

    REQUIRE( std::stoi(wex::style(doc.document_element(), "").number()) == 2);
    REQUIRE( wex::style(doc.document_element(), "").value() == "fore:#ffab8f");
    REQUIRE( std::stoi(wex::style(doc.document_element(), "cpp").number()) == 2);
    REQUIRE( wex::style(doc.document_element(), "").value() == "fore:#ffab8f");

    REQUIRE( doc.load_string("<style no = \"2\">styledefault+comment</style>"));
    
    REQUIRE( wex::style(doc.document_element(), "cpp").value().find("default") == std::string::npos);
    REQUIRE( wex::style(doc.document_element(), "cpp").value().find("comment") == std::string::npos);
    REQUIRE( wex::style(doc.document_element(), "cpp").value().find("+") == std::string::npos);
  }
  
  SUBCASE("apply")
  {
    wex::style style("mark_circle", "0");
    style.apply(get_stc());
    REQUIRE( style.is_ok());
    REQUIRE(!style.contains_default_style());
  
    wex::style().apply(get_stc());
  }
}
