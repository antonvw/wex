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

void fixture::testStyle()
{
  CPPUNIT_ASSERT(!wxExStyle().IsOk() );
  
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
    const wxExStyle test(
      style.first.first, style.second.first, style.second.second);
    
    if (!style.first.second.empty())
    {
      CPPUNIT_ASSERT( test.IsOk());
      CPPUNIT_ASSERT( test.GetNo() == style.first.second);
      CPPUNIT_ASSERT( test.GetValue() == style.second.first);
    }
    else
    {
      CPPUNIT_ASSERT(!test.IsOk());
    }
  }

  wxExSTC* stc = new wxExSTC(m_Frame, "hello stc");
  wxExStyle style("mark_circle", "0");
  style.Apply(stc);
  CPPUNIT_ASSERT( style.IsOk());
  CPPUNIT_ASSERT(!style.ContainsDefaultStyle());
}
