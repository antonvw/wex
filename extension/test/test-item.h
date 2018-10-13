////////////////////////////////////////////////////////////////////////////////
// Name:      test-item.h
// Purpose:   Declaration and implementation of TestItems
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/extension/item.h>

// Returns a notebook item (no subnotebook yet).
const auto NotebookItem(
  wex::itemtype style = wex::ITEM_NOTEBOOK, 
  wex::labeltype label = wex::LABEL_LEFT, 
  wxImageList* il = nullptr)
{
  return wex::item("notebook", {
    {"strings", 
     {{"string1", "first"},
      {"string2"},
      {"string3"}}},
    {"more-strings", 
     {{"string1", "nice"},
      {"string2"},
      {"string3"}}},
    {"checkboxes", 
     {{"checkbox1", wex::ITEM_CHECKBOX},
      {"checkbox2", wex::ITEM_CHECKBOX},
      {"checkbox3", wex::ITEM_CHECKBOX},
      {"checkbox4", wex::ITEM_CHECKBOX}}},
    {"spins", 
      {{"spin1", 0, 10},
       {"spin2", 0, 10},
       {"spin3", 0, 10},
       {"spin control double", 10.1, 15.0, 11.0, 0.1}}}},
    style, 0, 1, wex::control_data(), label, il);
};

/// Returns a vector with some items.
const auto TestItems()
{
  wxArrayString as;
  as.push_back("test1");
  as.push_back("test2");
  as.push_back("test3");

  return std::vector<wex::item> {
    {},
    {20},
    {wxHORIZONTAL},
    {"string1"},
    {"string2"},
    {"string3"},
    {"slider1", 10, 15, 10, wex::ITEM_SLIDER},
    {"slider2", 10, 15, 10, wex::ITEM_SLIDER},
    {NotebookItem()},
    {"button1", wex::ITEM_BUTTON},
    {"button2", wex::ITEM_BUTTON},
    {"combobox", wex::ITEM_COMBOBOX, as}};
}
