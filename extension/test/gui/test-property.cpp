////////////////////////////////////////////////////////////////////////////////
// Name:      test-property.cpp
// Purpose:   Implementation for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/property.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/stc.h>
#include "test.h"

void wxExGuiTestFixture::testProperty()
{
  wxExProperty inv;
  CPPUNIT_ASSERT( !inv.IsOk() );
  
  wxExProperty prop("man", "ugly");
  
  CPPUNIT_ASSERT( prop.IsOk());
  
  wxExSTC* stc = new wxExSTC(m_Frame, "hello stc");
  
  prop.Apply(stc);
  CPPUNIT_ASSERT( prop.IsOk());
  
  prop.ApplyReset(stc);
  CPPUNIT_ASSERT( prop.IsOk());
  
  CPPUNIT_ASSERT( prop.GetName() == "man");
  CPPUNIT_ASSERT( prop.GetValue() == "ugly");
}
