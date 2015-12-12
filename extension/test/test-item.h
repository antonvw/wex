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
const auto TestItems()
{
  wxArrayString as;
  as.push_back("test1");
  as.push_back("test2");
  as.push_back("test3");

  return std::vector<wxExItem> {
    wxExItem(),
    wxExItem(20),
    wxExItem(wxHORIZONTAL),
    wxExItem("string1"),
    wxExItem("string2"),
    wxExItem("string3"),
    wxExItem("slider1", 10, 15, 10, ITEM_SLIDER),
    wxExItem("slider2", 10, 15, 10, ITEM_SLIDER),
    wxExItem("notebook", wxExItem::ItemsNotebook {
      {"strings", 
        {wxExItem("string1"),
         wxExItem("string2"),
         wxExItem("string3")}},
      {"checkboxes", 
       {wxExItem("checkbox1", ITEM_CHECKBOX),
        wxExItem("checkbox2", ITEM_CHECKBOX),
        wxExItem("checkbox3", ITEM_CHECKBOX),
        wxExItem("checkbox4", ITEM_CHECKBOX)}},
      {"spins", 
        {wxExItem("spin1", 0, 10),
         wxExItem("spin2", 0, 10),
         wxExItem("spin3", 0, 10),
         wxExItem("spin control double", 10.1, 15.0, 11.0, 0.1)}}}, ITEM_NOTEBOOK_LIST, 0, 0, 1),
    wxExItem("button1", ITEM_BUTTON),
    wxExItem("button2", ITEM_BUTTON),
    wxExItem("combobox", ITEM_COMBOBOX, as)};
}
