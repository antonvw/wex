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
#include <wx/extension/stc.h>
#include "test.h"

void wxExGuiTestFixture::testVariable()
{
  wxExSTC* stc = new wxExSTC(wxTheApp->GetTopWindow(), "hello again");
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
  CPPUNIT_ASSERT( var.GetValue().IsEmpty());
  CPPUNIT_ASSERT(!var.Expand(ex));
  CPPUNIT_ASSERT(!var.IsModified());
  CPPUNIT_ASSERT(!var.IsInput());
  
  xml.DeleteAttribute("name");
  xml.AddAttribute("name", "Year");
  
  wxExVariable var2(&xml);
  CPPUNIT_ASSERT( var2.GetName() == "Year");
  CPPUNIT_ASSERT( var2.Expand(ex));
  CPPUNIT_ASSERT(!var2.IsModified());
  CPPUNIT_ASSERT(!var2.IsInput());
  
  wxExVariable var3("added");
  CPPUNIT_ASSERT( var3.GetName() == "added");
  CPPUNIT_ASSERT( var3.IsInput());
  var.SkipInput();
  // This is input, we cannot test it at this moment.
}
