////////////////////////////////////////////////////////////////////////////////
// Name:      test-variable.cpp
// Purpose:   Implementation for wxExtension cpp unit testing
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

void fixture::testVariable()
{
  wxExSTC* stc = new wxExSTC(m_Frame, "hello again");
  wxExEx* ex = new wxExEx(stc);
  
  for (auto it : std::vector<std::pair<char*, int>> {
    {"Created", wxExVariable::VARIABLE_BUILTIN},     
    {"HOME", wxExVariable::VARIABLE_ENVIRONMENT}, 
    {"aa", wxExVariable::VARIABLE_READ},
//    {"template", wxExVariable::VARIABLE_TEMPLATE},
    {"cc", wxExVariable::VARIABLE_INPUT},       
    {"dd", wxExVariable::VARIABLE_INPUT_ONCE},
    {"ee", wxExVariable::VARIABLE_INPUT_SAVE}})
  {
    wxExVariable v(it.first, "cht.txt", "zzz", it.second, false);
    CPPUNIT_ASSERT_MESSAGE( it.first, v.Expand(ex));
    CPPUNIT_ASSERT( v.GetName() == it.first);
    CPPUNIT_ASSERT(!v.IsModified());
    if (it.second >= wxExVariable::VARIABLE_INPUT && it.second <= wxExVariable::VARIABLE_INPUT_SAVE)
      CPPUNIT_ASSERT( v.IsInput());
    else
      CPPUNIT_ASSERT(!v.IsInput());
  }
  
  wxXmlNode xml(wxXML_ELEMENT_NODE, "variable");
  xml.AddAttribute("name", "test");
  xml.AddAttribute("type", "BUILTIN");
    
  wxExVariable var(&xml);
  CPPUNIT_ASSERT( var.GetName() == "test");
  CPPUNIT_ASSERT( var.GetValue().empty());
  CPPUNIT_ASSERT(!var.Expand(ex));
  CPPUNIT_ASSERT(!var.IsModified());
  CPPUNIT_ASSERT(!var.IsInput());
  
  xml.DeleteAttribute("name");
  
  // Test all builtin macro variables.
  for (const auto& it : m_BuiltinVariables)
  {
    xml.AddAttribute("name", it);

    wxExVariable var2(&xml);
    CPPUNIT_ASSERT( var2.GetName() == it);
    CPPUNIT_ASSERT( var2.GetValue().empty());
    CPPUNIT_ASSERT( var2.Expand(ex));
    wxString content;
    CPPUNIT_ASSERT( var2.Expand(ex, content));

    if (it == "Year")
    {
      CPPUNIT_ASSERT( content.StartsWith("20")); // start of year
    }
    
    CPPUNIT_ASSERT(!var2.IsModified());
    CPPUNIT_ASSERT(!var2.IsInput());
    
    xml.DeleteAttribute("name");
  }
    
  wxExVariable var3("added");
  CPPUNIT_ASSERT( var3.GetName() == "added");
  CPPUNIT_ASSERT( var3.IsInput());
  var.SkipInput();
  // This is input, we cannot test it at this moment.
  var.AskForInput();
  
  var3.Save(&xml);
}
