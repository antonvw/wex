////////////////////////////////////////////////////////////////////////////////
// Name:      test-listitem.cpp
// Purpose:   Implementation for wxExtension unit testing
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

TEST_CASE("wxExListItem")
{
  wxExListView* listView = new wxExListView(GetFrame(), wxExListView::LIST_FILE);
  AddPane(GetFrame(), listView);
  
  wxExListView* listView2 = new wxExListView(GetFrame(), wxExListView::LIST_NONE);
  AddPane(GetFrame(), listView2);
  
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

  INFO(std::to_string(add));
  REQUIRE(add < 5000);
  
  INFO(wxString::Format(
    "wxExListItem::Insert %d items in %ld ms", 3 * max, add).ToStdString());
  
  sw.Start();
  
  // The File Name column must be translated, otherwise
  // test fails.
  listView->SortColumn(_("File Name"), SORT_ASCENDING);
  
  const long sort = sw.Time();
  
  REQUIRE(sort < 2000);
  
  INFO(wxString::Format(
    "wxExListView::Sort %d items in %ld ms", 3 * max, sort).ToStdString());
    
  REQUIRE(listView->GetItemText(0, _("File Name")).Contains("main.cpp"));
  
  wxExListItem item(listView, wxExFileName("./test.h"));
  item.Insert();
  REQUIRE( item.GetFileName().GetFullName() == "test.h");
  REQUIRE( item.GetFileSpec().empty());
  REQUIRE( wxExListItem(listView, 
    wxExFileName("./test.h"), "*.txt").GetFileSpec() == "*.txt");
  REQUIRE( item.GetListView() == listView);
  REQUIRE(!item.IsReadOnly());
  
  item.SetItem("xx", "yy");
  item.Update();
  item.Delete();
}
