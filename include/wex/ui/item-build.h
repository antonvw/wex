////////////////////////////////////////////////////////////////////////////////
// Name:      item-build.h
// Purpose:   Methods to build items
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <vector>

#include <wex/ui/item.h>

namespace wex
{
/// Returns a vector of items to show a header from
/// specified vector of names.
const std::vector<item> add_header(const std::vector<std::string>& names);
}; // namespace wex
