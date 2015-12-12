////////////////////////////////////////////////////////////////////////////////
// Name:      test-listview.cpp
// Purpose:   Implementation for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/artprov.h> // for wxArt
#include <wx/extension/listview.h>
#include <wx/extension/managedframe.h>
#include "test.h"

void fixture::testListView()
{
  wxExListView* listView = new wxExListView(m_Frame, wxExListView::LIST_NONE);
  AddPane(m_Frame, listView);
  
  wxExListView::ConfigDialog(m_Frame, "test listview", wxAPPLY | wxCANCEL);
  
  CPPUNIT_ASSERT(listView->GetImageType() == wxExListView::IMAGE_ART);
  
  listView->ConfigGet();
  listView->SetSingleStyle(wxLC_REPORT);
  
  wxExColumn intcol(wxExColumn("Int", wxExColumn::COL_INT));
  CPPUNIT_ASSERT(!intcol.GetIsSortedAscending());
  intcol.SetIsSortedAscending(SORT_ASCENDING);
  CPPUNIT_ASSERT( intcol.GetIsSortedAscending());
  
  CPPUNIT_ASSERT(listView->AppendColumn(intcol) == 0);
  CPPUNIT_ASSERT(listView->AppendColumn(wxExColumn("Date", wxExColumn::COL_DATE)) == 1);
  CPPUNIT_ASSERT(listView->AppendColumn(wxExColumn("Float", wxExColumn::COL_FLOAT)) == 2);
  CPPUNIT_ASSERT(listView->AppendColumn(wxExColumn("String", wxExColumn::COL_STRING)) == 3);

  CPPUNIT_ASSERT(listView->FindColumn("Int") == 0);
  CPPUNIT_ASSERT(listView->FindColumn("Date") == 1);
  CPPUNIT_ASSERT(listView->FindColumn("Float") == 2);
  CPPUNIT_ASSERT(listView->FindColumn("String") == 3);
  
  listView->InsertItem(0, "test");
  
  CPPUNIT_ASSERT(listView->FindNext("test"));
  
  CPPUNIT_ASSERT(listView->ItemFromText("a new item"));
  CPPUNIT_ASSERT(listView->FindNext("a new item"));
  
  CPPUNIT_ASSERT( listView->ItemToText(0) == "test");
  CPPUNIT_ASSERT(!listView->ItemToText(-1).empty());
  
  //listView->Print(); // waits for input
  //listView->PrintPreview();

  // Delete all items, to test sorting later on.  
  listView->DeleteAllItems();
  
  listView->ItemsUpdate();
  
  for (int i = 0; i < 10; i++)
  {
    listView->InsertItem(i, std::to_string(i));
    listView->SetItem(i, 1, wxDateTime::Now().FormatISOCombined(' '));
    listView->SetItem(i, 2, wxString::Format("%f", (float)i / 2.0));
    listView->SetItem(i, 3, wxString::Format("hello %d", i));
  }
  
  // Test sorting.
  CPPUNIT_ASSERT(!listView->SortColumn("xxx"));
  CPPUNIT_ASSERT( listView->SortColumn("Int", SORT_ASCENDING));
  CPPUNIT_ASSERT( listView->GetItemText(0, "Int") == "0");
  CPPUNIT_ASSERT( listView->GetItemText(1, "Int") == "1");
  CPPUNIT_ASSERT( listView->SortColumn("Int", SORT_DESCENDING));
  CPPUNIT_ASSERT( listView->GetItemText(0, "Int") == "9");
  CPPUNIT_ASSERT( listView->GetItemText(1, "Int") == "8");

  CPPUNIT_ASSERT( listView->SortColumn("Date"));
  CPPUNIT_ASSERT( listView->SortColumn("Float"));
  CPPUNIT_ASSERT( listView->SortColumn("String"));
  
  CPPUNIT_ASSERT( listView->GetSortedColumnNo() == 3);
  listView->SortColumnReset();
  CPPUNIT_ASSERT( listView->GetSortedColumnNo() == -1);
  
  listView->SetItem(0, 1, "incorrect date");
  CPPUNIT_ASSERT(!listView->SortColumn("Date"));
  
  listView->SetItemImage(0, wxART_WARNING);
  
  wxExListView* listView2 = new wxExListView(m_Frame, wxExListView::LIST_FILE);
  AddPane(m_Frame, listView2);
  
  CPPUNIT_ASSERT( listView2->GetImageType() == wxExListView::IMAGE_FILE_ICON);
  CPPUNIT_ASSERT(!listView2->GetTypeDescription().empty());
  CPPUNIT_ASSERT(!listView2->wxExListView::GetTypeDescription(wxExListView::LIST_FILE).empty());
  
  CPPUNIT_ASSERT( listView2->ItemFromText("test.h\ntest.h"));
  CPPUNIT_ASSERT(!listView2->ItemToText(0).empty());
  CPPUNIT_ASSERT(!listView2->ItemToText(-1).empty());
  
  listView->ItemsUpdate();
  
  wxListEvent event(wxEVT_LIST_ITEM_ACTIVATED);
  
  for (auto id : std::vector<int> {0}) 
  {
    event.m_itemIndex = id;
    wxPostEvent(listView, event);
    wxPostEvent(listView2, event);
  }
}
