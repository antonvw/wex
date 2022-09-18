////////////////////////////////////////////////////////////////////////////////
// Name:      item-build.cpp
// Purpose:   Implementation of item build methods
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <algorithm>

#include <wex/ui/item-build.h>

const wex::item wex::add_combobox_with_max(
  const std::string& name_combo,
  const std::string& name_max,
  const data::item&  data)
{
  return {
    {"",
     {{name_combo, wex::item::COMBOBOX, std::any(), data},
      {name_max, -1, INT_MAX}}}};
}

const std::vector<wex::item>
wex::add_header(const std::vector<std::string>& names)
{
  std::vector<item> v;

  std::for_each(
    names.begin(),
    names.end(),
    [&v](const auto& name)
    {
      v.push_back(
        {name,
         std::string(),
         item::STATICTEXT,
         data::control().window(
           data::window().style(wxALIGN_CENTRE_HORIZONTAL))});
    });

  return v;
}
