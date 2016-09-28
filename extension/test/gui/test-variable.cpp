////////////////////////////////////////////////////////////////////////////////
// Name:      test-variable.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/xml/xml.h>
#include <wx/extension/variable.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/stc.h>
#include "test.h"

TEST_CASE("wxExVariable", "[stc][vi]")
{
  wxExEx* ex = new wxExEx(GetSTC());

  for (auto it : std::vector<std::pair<char*, int>> {
    {"Created", wxExVariable::VARIABLE_BUILTIN},     
#ifdef __UNIX__
    {"HOME", wxExVariable::VARIABLE_ENVIRONMENT}, 
#endif
    {"aa", wxExVariable::VARIABLE_READ},
//    {"template", wxExVariable::VARIABLE_TEMPLATE},
    {"cc", wxExVariable::VARIABLE_INPUT},       
    {"dd", wxExVariable::VARIABLE_INPUT_ONCE},
    {"ee", wxExVariable::VARIABLE_INPUT_SAVE}})
  {
    wxExVariable v(it.first, "cht.txt", "zzz", it.second, false);
    INFO( it.first);
    REQUIRE( v.Expand(ex));
    REQUIRE( v.GetName() == it.first);
    REQUIRE(!v.IsModified());
    if (it.second >= wxExVariable::VARIABLE_INPUT && it.second <= wxExVariable::VARIABLE_INPUT_SAVE)
      REQUIRE( v.IsInput());
    else
      REQUIRE(!v.IsInput());
  }

  wxXmlNode xml(wxXML_ELEMENT_NODE, "variable");
  xml.AddAttribute("name", "test");
  xml.AddAttribute("type", "BUILTIN");
    
  wxExVariable var(&xml);
  REQUIRE( var.GetName() == "test");
  REQUIRE( var.GetValue().empty());
  REQUIRE(!var.Expand(ex));
  REQUIRE(!var.IsModified());
  REQUIRE(!var.IsInput());
  
  xml.DeleteAttribute("name");
  
  // Test all builtin macro variables.
  for (const auto& it : GetBuiltinVariables())
  {
    xml.AddAttribute("name", it);

    wxExVariable var2(&xml);
    REQUIRE( var2.GetName() == it);
    REQUIRE( var2.GetValue().empty());
    REQUIRE( var2.Expand(ex));
    std::string content;
    REQUIRE( var2.Expand(ex, content));

    if (it == "Year")
    {
      REQUIRE( wxString(content).StartsWith("20")); // start of year
    }
    
    REQUIRE(!var2.IsModified());
    REQUIRE(!var2.IsInput());
    
    xml.DeleteAttribute("name");
  }
    
  wxExVariable var3("added");
  REQUIRE( var3.GetName() == "added");
  REQUIRE( var3.IsInput());
  var.SkipInput();
  // This is input, we cannot test it at this moment.
  var.AskForInput();
  
  var3.Save(&xml);
}
