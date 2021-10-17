////////////////////////////////////////////////////////////////////////////////
// Name:      menubar.h
// Purpose:   Declaration of wex::menubar class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019-2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/ui/menu.h>

namespace wex
{
/// Offers a menubar to hold menus.
class menubar : public wxMenuBar
{
public:
  /// Constructor.
  explicit menubar(const std::vector<std::pair<menu*, std::string>>& menus)
  {
    for (const auto& it : menus)
    {
      if (it.first != nullptr)
      {
        Append(it.first, it.second);
      }
    }
  }
};
}; // namespace wex
