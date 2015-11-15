////////////////////////////////////////////////////////////////////////////////
// Name:      test-item.h
// Purpose:   Declaration and implementation of TestItems
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/extension/item.h>
#include <wx/extension/listview.h>

/// Returns a vector with all items available.
const auto TestItems(bool use_notebook = false)
{
  wxArrayString as;
  as.push_back("test1");
  as.push_back("test2");
  as.push_back("test3");
  
  if (use_notebook)
  {
    return std::vector<wxExItem> {
      wxExItem("notebook", wxEmptyString, wxEmptyString, "page1"),
      wxExItem(20, "page1"),
      wxExItem(wxHORIZONTAL, "page1"),
      wxExItem()};
  }
  
  return std::vector<wxExItem> {
    wxExItem(),
    wxExItem(20),
    wxExItem(wxHORIZONTAL),
    wxExItem("string"),
    wxExItem("spin control", 10, 5, 15),
    wxExItem("spin control double", 10.1, 5.0, 15.0, wxEmptyString, 0.1),
    wxExItem("slider", 10, 5, 15, wxEmptyString, ITEM_SLIDER),
    wxExItem("checkbox1", ITEM_CHECKBOX),
    wxExItem("checkbox2", ITEM_CHECKBOX),
    wxExItem("checkbox3", ITEM_CHECKBOX),
    wxExItem("listview-folder", ITEM_LISTVIEW),
    wxExItem("listview-file", ITEM_LISTVIEW, wxAny(), wxEmptyString, false, wxID_ANY, true, (long)wxExListView::LIST_FILE),
    wxExItem("listview-none", ITEM_LISTVIEW, wxString("1\n2\n3\n4\n"), wxEmptyString, false, wxID_ANY, true, (long)wxExListView::LIST_NONE),
    wxExItem("button", ITEM_BUTTON),
    wxExItem("STC", "cpp", wxEmptyString, wxEmptyString, 0, ITEM_STC),
    wxExItem("combobox", ITEM_COMBOBOX, as)};
}
