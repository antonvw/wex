////////////////////////////////////////////////////////////////////////////////
// Name:      test-frame.cpp
// Purpose:   Implementation for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/menu.h>
#include <wx/extension/filehistory.h>
#include <wx/extension/managedframe.h>
#include "test.h"

void fixture::testFileHistory()
{
  wxExFileHistory history;
  
  CPPUNIT_ASSERT( history.GetCount() == 0);
  
  CPPUNIT_ASSERT( history.SetRecentFile("xxx.cpp"));
  CPPUNIT_ASSERT(!history.SetRecentFile(""));
  CPPUNIT_ASSERT( history.GetCount() > 0);
  CPPUNIT_ASSERT( history.GetRecentFile() == "xxx.cpp");
  
  history.Clear();
  CPPUNIT_ASSERT( history.GetCount() == 0);
  CPPUNIT_ASSERT( history.GetRecentFile().empty());
  
  wxMenu* menu = new wxMenu();

  history.PopupMenu(m_Frame, 1, 5);
  
  history.UseMenu(100, menu);
  history.Save();
}
