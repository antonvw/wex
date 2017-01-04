////////////////////////////////////////////////////////////////////////////////
// Name:      test-style.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <numeric>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/style.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/stc.h>
#include "test.h"

TEST_CASE("wxExStyle")
{
  SUBCASE("Default constructor")
  {
    REQUIRE(!wxExStyle().IsOk() );
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
      const wxExStyle test(
        style.first.first, style.second.first, style.second.second);
      
      if (!style.first.second.empty())
      {
        REQUIRE( test.IsOk());
        REQUIRE( test.GetNo() == style.first.second);
        REQUIRE( test.GetValue() == style.second.first);
      }
      else
      {
        REQUIRE(!test.IsOk());
      }
    }
  }

  SUBCASE("Constructor using xml node")
  {
    pugi::xml_document doc;
    REQUIRE( doc.load_string("<style no = \"2\">string</style>"));

    REQUIRE( std::stoi(wxExStyle(doc.document_element(), "").GetNo()) == 2);
    REQUIRE( wxExStyle(doc.document_element(), "").GetValue() == "fore:blue");
    REQUIRE( std::stoi(wxExStyle(doc.document_element(), "cpp").GetNo()) == 2);
    REQUIRE( wxExStyle(doc.document_element(), "").GetValue() == "fore:blue");

    REQUIRE( doc.load_string("<style no = \"2\">styledefault+comment</style>"));
    
    REQUIRE( wxExStyle(doc.document_element(), "cpp").GetValue().find("default") == std::string::npos);
    REQUIRE( wxExStyle(doc.document_element(), "cpp").GetValue().find("comment") == std::string::npos);
    REQUIRE( wxExStyle(doc.document_element(), "cpp").GetValue().find("+") == std::string::npos);
  }
  
  SUBCASE("Apply")
  {
    wxExStyle style("mark_circle", "0");
    style.Apply(GetSTC());
    REQUIRE( style.IsOk());
    REQUIRE(!style.ContainsDefaultStyle());
  
    wxExStyle().Apply(GetSTC());
  }
}
