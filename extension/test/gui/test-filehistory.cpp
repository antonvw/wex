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
  
  history.AddFileToHistory("xxx.cpp");
  history.AddFileToHistory("");
  CPPUNIT_ASSERT( history.GetCount() == 0);
  CPPUNIT_ASSERT( history.GetHistoryFile().empty());
  
  history.AddFileToHistory(GetTestFile().GetFullPath());
  CPPUNIT_ASSERT( history.GetCount() == 1);
  CPPUNIT_ASSERT( history.GetVector(0).size() == 0);
  CPPUNIT_ASSERT( history.GetVector(5).size() == 1);
  
  history.Clear();
  CPPUNIT_ASSERT( history.GetCount() == 0);
  CPPUNIT_ASSERT( history.GetHistoryFile().empty());
  CPPUNIT_ASSERT( history.GetHistoryFile(100).empty());
  
  wxMenu* menu = new wxMenu();
  menu->Append(1, "x");
  menu->Append(2, "y");

  history.PopupMenu(m_Frame, 5);
  
  history.UseMenu(100, menu);
  
  history.Save();
  
  wxExFileHistory history2(4, 1000, "MY-KEY");
  history2.AddFileToHistory(GetTestFile().GetFullPath());
  CPPUNIT_ASSERT( history2.GetCount() == 1);
  history2.Save();
}
