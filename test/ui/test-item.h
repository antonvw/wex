////////////////////////////////////////////////////////////////////////////////
// Name:      test-item.h
// Purpose:   Declaration and implementation of test_items
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/ui/item.h>

// Returns a notebook item (no subnotebook yet).
const auto test_notebook_item(
  wex::item::type_t        style = wex::item::NOTEBOOK,
  wex::data::item::label_t label = wex::data::item::LABEL_LEFT,
  wxImageList*             il    = nullptr)
{
  return wex::item(
    "notebook",
    {{"strings", {{"string1", "first"}, {"string2"}, {"string3"}}},
     {"more-strings", {{"string4", "nice"}, {"string5"}, {"string6"}}},
     {"checkboxes",
      {{"checkbox1", wex::item::CHECKBOX},
       {"checkbox2", wex::item::CHECKBOX},
       {"checkbox3", wex::item::CHECKBOX},
       {"checkbox4", wex::item::CHECKBOX}}},
     {"spins",
      {{"spin1", 0, 10},
       {"spin2", 0, 10},
       {"spin3", 0, 10},
       {"spin control double", 10.1, 15.0, 11.0, wex::data::item().inc(0.1)}}}},
    style,
    wex::data::item().label_type(label).image_list(il));
};

/// Returns a vector with some items.
const auto test_items()
{
  return std::vector<wex::item>{
    {},
    {20},
    {wxHORIZONTAL},
    {"string1"},
    {"string2"},
    {"string3"},
    {"slider1", 10, 15, 10, wex::item::SLIDER},
    {"slider2", 10, 15, 10, wex::item::SLIDER},
    {test_notebook_item()},
    {"button1", wex::item::BUTTON},
    {"button2", wex::item::BUTTON},
    {"combobox",
     wex::item::COMBOBOX,
     std::list<std::string>{"test1", "test2", "test3"}}};
}
