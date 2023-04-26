////////////////////////////////////////////////////////////////////////////////
// Name:      test-item.h
// Purpose:   Declaration and implementation of class test_item
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/core/types.h>
#include <wex/ui/item.h>

namespace wex
{
class test_item
{
public:
  // Returns a notebook item (no subnotebook yet).
  const wex::item notebook(
    wex::item::type_t        style = wex::item::NOTEBOOK,
    wex::data::item::label_t label = wex::data::item::LABEL_LEFT,
    wxImageList*             il    = nullptr);

  /// Returns a vector with some items.
  const std::vector<wex::item> vector();
};

// inline implementation

inline const wex::item wex::test_item::notebook(
  wex::item::type_t        style,
  wex::data::item::label_t label,
  wxImageList*             il)
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

inline const std::vector<wex::item> wex::test_item::vector()
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
    {notebook()},
    {"button1", wex::item::BUTTON},
    {"button2", wex::item::BUTTON},
    {"combobox",
     wex::item::COMBOBOX,
     wex::strings_t{"test1", "test2", "test3"}}};
}
} // namespace wex
