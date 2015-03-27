////////////////////////////////////////////////////////////////////////////////
// Name:      test-indicator.cpp
// Purpose:   Implementation for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/xml/xml.h>
#include <wx/extension/indicator.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/stc.h>
#include "test.h"

void fixture::testIndicator()
{
  wxExIndicator ind;
  CPPUNIT_ASSERT(!ind.IsOk() );
  
  wxExIndicator indx(5, 2);
  wxExIndicator indy(7, 5);
  
  CPPUNIT_ASSERT( indx.IsOk());
  CPPUNIT_ASSERT( indy.IsOk());
  CPPUNIT_ASSERT( indx < indy );
  
  wxXmlNode xml(wxXML_ELEMENT_NODE, "indicator");
  xml.AddAttribute("no", "5");
  xml.SetContent("symbol,green");

  wxExIndicator ind2(&xml);
  CPPUNIT_ASSERT( ind2.GetNo() == 5);
  CPPUNIT_ASSERT( ind2.IsOk());
  
  wxExSTC* stc = new wxExSTC(m_Frame, "hello stc");
  ind2.Apply(stc);
  CPPUNIT_ASSERT( ind2.IsOk());
}
