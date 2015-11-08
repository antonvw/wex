////////////////////////////////////////////////////////////////////////////////
// Name:      test-Item.cpp
// Purpose:   Implementation for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/item.h>
#include <wx/extension/managedframe.h>
#include "../test-item.h"
#include "test.h"

void fixture::testItem()
{
  wxPanel* panel = new wxPanel(m_Frame);
  AddPane(m_Frame, panel);
  wxGridSizer* sizer = new wxGridSizer(3);
  panel->SetSizer(sizer);
  
  wxExItem item("item", "hello string", "info1", "page1", 1, ITEM_STRING, true, true, 2);
  
  CPPUNIT_ASSERT( item.GetColumns() == -1); // page cols
  CPPUNIT_ASSERT( item.GetInitial().As<wxString>() == "hello string");
  CPPUNIT_ASSERT( item.GetIsRequired());
  CPPUNIT_ASSERT( item.GetLabel() == "item");
  CPPUNIT_ASSERT( item.GetPage() == "page1");
  CPPUNIT_ASSERT( item.GetType() == ITEM_STRING);
  CPPUNIT_ASSERT( item.GetWindow() == NULL);
  CPPUNIT_ASSERT( item.GetValue().IsNull());
  CPPUNIT_ASSERT(!item.IsRowGrowable());
  
  item.SetValidator(NULL);
  
  // setting value if window is NULL should have no effect.
  CPPUNIT_ASSERT(!item.SetValue(wxString("test")));
  CPPUNIT_ASSERT( item.GetValue().IsNull());
  
  item.SetRowGrowable(true);
  CPPUNIT_ASSERT( item.IsRowGrowable());
  
  wxExItem item_int("int", ITEM_INT, "100");
  CPPUNIT_ASSERT( item_int.GetType() == ITEM_INT);
  
  wxExItem item_spin("spindouble", 25.0, 20.0, 30.0, wxEmptyString, 0.1);
  CPPUNIT_ASSERT( item_spin.GetType() == ITEM_SPINCTRL_DOUBLE);
  
  wxExItem item_picker("picker", ITEM_FILEPICKERCTRL, "/usr/bin/git");
  
#if wxCHECK_VERSION(3,1,0)
  item.Layout(panel, sizer);
  CPPUNIT_ASSERT( item.GetWindow() != NULL);
  CPPUNIT_ASSERT( item.GetValue() == "hello string");
  CPPUNIT_ASSERT( item.SetValue(wxString("value changed")));
  CPPUNIT_ASSERT( item.GetValue() == "value changed");
  CPPUNIT_ASSERT( item.GetInitial().As<wxString>() == "hello string");
  CPPUNIT_ASSERT( item.GetWindow()->GetWindowStyleFlag() == 1);
  
  item_int.Layout(panel, sizer);
  CPPUNIT_ASSERT( item_int.GetWindow() != NULL);
  CPPUNIT_ASSERT( item_int.GetValue() == 100);
  CPPUNIT_ASSERT( item_int.SetValue(300));
  CPPUNIT_ASSERT( item_int.GetValue() == 300);
  
  item_picker.Layout(panel, sizer);
  CPPUNIT_ASSERT( item_picker.GetValue() == "/usr/bin/git");
#endif
  
  std::vector <wxExItem> items {
    item, item_int, item_spin, item_picker};
  
  const auto more(TestItems());
  items.insert(items.end(), more.begin(), more.end());
  const auto more2(TestItems(true));
  items.insert(items.end(), more2.begin(), more2.end());
  
#if wxCHECK_VERSION(3,1,0)
  // Layout the items and check control is created.
  for (auto& it : items)
  {
    // ITEM_USER is not yet laid out ok, gives errors.
    if (it.GetType() != ITEM_USER)
    {
      // Testing on not NULL not possible,
      // not all items need a sizer.
      it.Layout(panel, sizer);
    }
 
    if (it.GetType() != ITEM_EMPTY && it.GetType() != ITEM_SPACER)
    {
      CPPUNIT_ASSERT(it.GetWindow() != NULL);
    }
  }
#endif
}
