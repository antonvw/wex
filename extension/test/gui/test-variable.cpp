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

void wxExGuiTestFixture::testVariable()
{
  wxExSTC* stc = new wxExSTC(m_Frame, "hello again");
  wxExEx* ex = new wxExEx(stc);
  
  wxExVariable v;
  CPPUNIT_ASSERT( v.Expand(ex));
  CPPUNIT_ASSERT( v.GetName().empty());
  CPPUNIT_ASSERT(!v.IsModified());
  CPPUNIT_ASSERT(!v.IsInput());
  
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
