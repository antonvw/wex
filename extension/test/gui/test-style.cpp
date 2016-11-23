////////////////////////////////////////////////////////////////////////////////
// Name:      test-style.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <numeric>
#include <wx/wxprec.h>
#include <wx/xml/xml.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/style.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/stc.h>
#include "test.h"

TEST_CASE("wxExStyle", "[stc][lexer]")
{
  SECTION("Default constructor")
  {
    REQUIRE(!wxExStyle().IsOk() );
  }
  
  SECTION("Constructor using no and value")
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

  SECTION("Constructor using xml node")
  {
    wxXmlNode xml(wxXML_ELEMENT_NODE, "style");
    xml.AddAttribute("no", "2");
    new wxXmlNode(&xml, wxXML_TEXT_NODE , "", "string");

    REQUIRE( std::stoi(wxExStyle(&xml, "").GetNo()) == 2);
    REQUIRE( wxExStyle(&xml, "").GetValue() == "fore:blue");
    REQUIRE( std::stoi(wxExStyle(&xml, "cpp").GetNo()) == 2);
    REQUIRE( wxExStyle(&xml, "").GetValue() == "fore:blue");

    wxXmlNode xml2(wxXML_ELEMENT_NODE, "style");
    xml2.AddAttribute("no", "2");
    new wxXmlNode(&xml2, wxXML_TEXT_NODE , "", "styledefault+comment");

    REQUIRE( wxExStyle(&xml2, "cpp").GetValue().find("default") == std::string::npos);
    REQUIRE( wxExStyle(&xml2, "cpp").GetValue().find("comment") == std::string::npos);
    REQUIRE( wxExStyle(&xml2, "cpp").GetValue().find("+") == std::string::npos);
  }
  
  SECTION("Apply")
  {
    wxExStyle style("mark_circle", "0");
    style.Apply(GetSTC());
    REQUIRE( style.IsOk());
    REQUIRE(!style.ContainsDefaultStyle());
  
    wxExStyle().Apply(GetSTC());
  }
}
