////////////////////////////////////////////////////////////////////////////////
// Name:      test-listitem.cpp
// Purpose:   Implementation for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/listitem.h>
#include <wx/extension/managedframe.h>
#include "test.h"

void fixture::testListItem()
{
  wxExListView* listView = new wxExListView(m_Frame, wxExListView::LIST_FILE);
  AddPane(m_Frame, listView);
  
  wxExListView* listView2 = new wxExListView(m_Frame, wxExListView::LIST_NONE);
  AddPane(m_Frame, listView2);
  
  wxStopWatch sw;
  sw.Start();

  const int max = 250;
  for (int j = 0; j < max; j++)
  {
    wxExListItem item1(listView, wxExFileName("./test.h"));
    item1.Insert();
    wxExListItem item2(listView, wxExFileName("./test.cpp"));
    item2.Insert();
    wxExListItem item3(listView, wxExFileName("./main.cpp"));
    item3.Insert();
    wxExListItem item4(listView2, wxExFileName("./test.h"));
    item4.Insert();
  }

  const long add = sw.Time();

  CPPUNIT_ASSERT_MESSAGE(std::to_string(add), add < 5000);
  
  Report(wxString::Format(
    "wxExListItem::Insert %d items in %ld ms", 3 * max, add).ToStdString());
  
  sw.Start();
  
  // The File Name column must be translated, otherwise
  // test fails.
  listView->SortColumn(_("File Name"), SORT_ASCENDING);
  
  const long sort = sw.Time();
  
  CPPUNIT_ASSERT(sort < 2000);
  
  Report(wxString::Format(
    "wxExListView::Sort %d items in %ld ms", 3 * max, sort).ToStdString());
    
  CPPUNIT_ASSERT(listView->GetItemText(0, _("File Name")).Contains("main.cpp"));
  
  wxExListItem item(listView, wxExFileName("./test.h"));
  item.Insert();
  CPPUNIT_ASSERT( item.GetFileName().GetFullPath() == "./test.h");
  CPPUNIT_ASSERT( item.GetFileSpec().empty());
  CPPUNIT_ASSERT( wxExListItem(listView, 
    wxExFileName("./test.h"), "*.txt").GetFileSpec() == "*.txt");
  CPPUNIT_ASSERT( item.GetListView() == listView);
  CPPUNIT_ASSERT(!item.IsReadOnly());
  
  item.SetItem("xx", "yy");
  item.Update();
  item.Delete();
}
