////////////////////////////////////////////////////////////////////////////////
// Name:      item-build.h
// Purpose:   Methods to build items
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <vector>

#include <wex/ui/item.h>

namespace wex
{
/// Returns a (group) item consisting of a combobox and a max.
const item add_combobox_with_max(
  const std::string& name_combo,
  const std::string& name_max,
  const data::item&  data = data::item());

/// Returns a vector of items to show a header from
/// specified vector of names.
const std::vector<item> add_header(const std::vector<std::string>& names);
}; // namespace wex
