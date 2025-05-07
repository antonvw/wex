////////////////////////////////////////////////////////////////////////////////
// Name:      item-build.cpp
// Purpose:   Implementation of item build methods
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <algorithm>

#include <wex/factory/control.h>
#include <wex/ui/frd.h>
#include <wex/ui/item-build.h>

const wex::item wex::add_combobox_with_max(
  const std::string& name_combo,
  const std::string& name_max,
  const data::item&  data)
{
  return {
    {"",
     {{name_combo, item::COMBOBOX, std::any(), data},
      {name_max, -1, INT_MAX}}}};
}

const wex::item wex::add_find_text(const data::item& data)
{
  data::item my_data(data);
  data::control cd(data.control());
  cd.is_required(true);
  my_data.control(cd);

  return item(find_replace_data::get()->text_find(),
    item::COMBOBOX,
    std::any(),
    my_data);
}

const std::vector<wex::item>
wex::add_header(const std::vector<std::string>& names)
{
  std::vector<item> v;

  std::ranges::for_each(
    names,
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
