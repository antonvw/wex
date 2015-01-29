////////////////////////////////////////////////////////////////////////////////
// Name:      test-style.cpp
// Purpose:   Implementation for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/style.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/stc.h>
#include "test.h"

void wxExGuiTestFixture::testStyle()
{
  wxExStyle inv;
  CPPUNIT_ASSERT(!inv.IsOk() );
  
  wxExStyle test1("MARK_CIRCLE", "ugly");
  wxExStyle test2("512", "ugly");
  wxExStyle test3("number,string,comment", "fore:blue", "cpp");
  wxExStyle test4("number,string,xxx", "fore:black", "cpp");
  wxExStyle test5("xxx", "fore:black", "cpp");
  
  CPPUNIT_ASSERT(!test1.IsOk());
  CPPUNIT_ASSERT(!test2.IsOk());
  CPPUNIT_ASSERT( test3.IsOk());
  CPPUNIT_ASSERT( test3.GetValue() == "fore:blue");
  CPPUNIT_ASSERT( test4.IsOk()); // because number, string is ok
  CPPUNIT_ASSERT(!test5.IsOk());
  
  wxExStyle style("mark_circle", "0");
  
  wxExSTC* stc = new wxExSTC(m_Frame, "hello stc");
  
  style.Apply(stc);
  CPPUNIT_ASSERT( style.IsOk());
  
  CPPUNIT_ASSERT(!style.ContainsDefaultStyle());
}
