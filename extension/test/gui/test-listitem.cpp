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

void wxExGuiTestFixture::testListItem()
{
  wxExListViewFileName* listView = new wxExListViewFileName(
    m_Frame, wxExListViewFileName::LIST_FILE);
  
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
  }
  
  const long add = sw.Time();

  CPPUNIT_ASSERT(add < 2000);
  
  Report(wxString::Format(
    "wxExListTiem::Insert %d items in %ld ms", 3 * max, add).ToStdString());
  
  sw.Start();
  
  // The File Name column must be translated, otherwise
  // test fails.
  listView->SortColumn(_("File Name"), SORT_ASCENDING);
  
  const long sort = sw.Time();
  
  CPPUNIT_ASSERT(sort < 1000);
  
  Report(wxString::Format(
    "wxExListView::Sort %d items in %ld ms", 3 * max, sort).ToStdString());
    
  CPPUNIT_ASSERT(listView->GetItemText(0, _("File Name")).Contains("main.cpp"));
}
