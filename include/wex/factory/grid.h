////////////////////////////////////////////////////////////////////////////////
// Name:      factory/grid.h
// Purpose:   Declaration of wex::factory::grid class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/grid.h>

namespace wex::factory
{
/// Offers interface to wxGrid.
class grid : public wxGrid
{
public:
  /// Destructor.
  virtual ~grid() = default;
};
}; // namespace wex::factory
