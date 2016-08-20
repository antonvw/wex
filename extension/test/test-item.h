////////////////////////////////////////////////////////////////////////////////
// Name:      test-item.h
// Purpose:   Declaration and implementation of TestItems
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/extension/item.h>

// Returns a notebook item (no subnotebook yet).
const auto NotebookItem(
  wxExItemType style = ITEM_NOTEBOOK, 
  wxExLabelType label = LABEL_LEFT, 
  wxImageList* il = nullptr)
{
  return wxExItem("notebook", {
    {"strings", 
     {{"string1", "first"},
      {"string2"},
      {"string3"}}},
    {"more-strings", 
     {{"string1", "nice"},
      {"string2"},
      {"string3"}}},
    {"checkboxes", 
     {{"checkbox1", ITEM_CHECKBOX},
      {"checkbox2", ITEM_CHECKBOX},
      {"checkbox3", ITEM_CHECKBOX},
      {"checkbox4", ITEM_CHECKBOX}}},
    {"spins", 
      {{"spin1", 0, 10},
       {"spin2", 0, 10},
       {"spin3", 0, 10},
       {"spin control double", 10.1, 15.0, 11.0, 0.1}}}},
    style, 0, 0, 1, label, il);
};

/// Returns a vector with some items.
const auto TestItems()
{
  wxArrayString as;
  as.push_back("test1");
  as.push_back("test2");
  as.push_back("test3");

  return std::vector<wxExItem> {
    {},
    {20},
    {wxHORIZONTAL},
    {"string1"},
    {"string2"},
    {"string3"},
    {"slider1", 10, 15, 10, ITEM_SLIDER},
    {"slider2", 10, 15, 10, ITEM_SLIDER},
    {NotebookItem()},
    {"button1", ITEM_BUTTON},
    {"button2", ITEM_BUTTON},
    {"combobox", ITEM_COMBOBOX, as}};
}
