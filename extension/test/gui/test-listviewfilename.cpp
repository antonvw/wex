////////////////////////////////////////////////////////////////////////////////
// Name:      test-listviewfilename.cpp
// Purpose:   Implementation for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/listview.h>
#include <wx/extension/managedframe.h>
#include "test.h"

void fixture::testListViewFileName()
{
  wxExListViewFileName* listView = new wxExListViewFileName(
    m_Frame, wxExListViewFileName::LIST_FILE);
  m_Frame->GetManager().AddPane(listView, 
    wxAuiPaneInfo().Bottom().Caption("ListViewFileName"));
  m_Frame->GetManager().Update();
  
  CPPUNIT_ASSERT(!listView->GetTypeDescription().empty());
  CPPUNIT_ASSERT(!listView->wxExListViewFileName::GetTypeDescription(
    wxExListViewFileName::LIST_FILE).empty());
  
  listView->SetSingleStyle(wxLC_REPORT);
  
  CPPUNIT_ASSERT( listView->ItemFromText("test.h"));
  CPPUNIT_ASSERT(!listView->ItemToText(0).empty());
  
  listView->ItemsUpdate();
  
  wxListEvent event(wxEVT_LIST_ITEM_ACTIVATED);
  
  for (auto id : std::vector<int> {0}) 
  {
    event.m_itemIndex = id;
    wxPostEvent(listView, event);
  }
}
