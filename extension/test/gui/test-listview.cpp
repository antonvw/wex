////////////////////////////////////////////////////////////////////////////////
// Name:      test-listview.cpp
// Purpose:   Implementation for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/artprov.h> // for wxArtID
#include <wx/extension/listview.h>
#include <wx/extension/managedframe.h>
#include "test.h"

void fixture::testListView()
{
  wxExListView* listView = new wxExListView(m_Frame);
  
  listView->SetSingleStyle(wxLC_REPORT);
  
  CPPUNIT_ASSERT(listView->AppendColumn(wxExColumn("Int", wxExColumn::COL_INT)) == 0);
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
  
  CPPUNIT_ASSERT(listView->ItemToText(0) == "test");
  
  //listView->Print(); // TODO: asserts
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
  
  listView->SetItem(0, 1, "incorrect date");
  CPPUNIT_ASSERT(!listView->SortColumn("Date"));
  
  listView->SetItemImage(0, wxART_WARNING);
}
