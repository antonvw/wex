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

  return use_notebook ? std::vector<wxExItem> {
    wxExItem("notebook", wxEmptyString, "page1"),
    wxExItem(20, "page1"),
    wxExItem(wxHORIZONTAL, "page1"),
    wxExItem("listview-folder", ITEM_LISTVIEW, wxEmptyString, "page2"),
    wxExItem("listview-file", ITEM_LISTVIEW, wxAny(), "page2", false, wxID_ANY, LABEL_LEFT, (long)wxExListView::LIST_FILE),
    wxExItem("listview-none", ITEM_LISTVIEW, ("1\n2\n3\n4\n"), "page2", false, wxID_ANY, LABEL_LEFT, (long)wxExListView::LIST_NONE),
    wxExItem("STC", "cpp", "page2", 0, ITEM_STC, false),
    wxExItem()}
    : std::vector<wxExItem> {
    wxExItem(),
    wxExItem(20),
    wxExItem(wxHORIZONTAL),
    wxExItem("string1"),
    wxExItem("string2"),
    wxExItem("string3"),
    wxExItem("spin control1", 10, 5, 15),
    wxExItem("spin control2", 10, 5, 15),
    wxExItem("spin control double", 10.1, 5.0, 15.0, wxEmptyString, 0.1),
    wxExItem("slider1", 10, 5, 15, wxEmptyString, ITEM_SLIDER),
    wxExItem("slider2", 10, 5, 15, wxEmptyString, ITEM_SLIDER),
    wxExItem("checkbox1", ITEM_CHECKBOX),
    wxExItem("checkbox2", ITEM_CHECKBOX),
    wxExItem("checkbox3", ITEM_CHECKBOX),
    wxExItem("checkbox4", ITEM_CHECKBOX),
    wxExItem(wxExItem::ItemsNotebook {
      {"page1", 
        {wxExItem("string1"),
         wxExItem("string2"),
         wxExItem("string3")}},
      {"page2", 
        {wxExItem("spin1", 5, 0, 10),
         wxExItem("spin2", 5, 0, 10),
         wxExItem("spin3", 5, 0, 10)}}}, ITEM_NOTEBOOK_LIST, 0, 0, 1),
    wxExItem("button1", ITEM_BUTTON),
    wxExItem("button2", ITEM_BUTTON),
    wxExItem("combobox", ITEM_COMBOBOX, as)};
}
