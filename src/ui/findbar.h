////////////////////////////////////////////////////////////////////////////////
// Name:      findbar.cpp
// Purpose:   Declaration of wex::find_bar class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2024-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/factory/window.h>
#include <wex/ui/ex-commandline.h>
#include <wex/ui/frame.h>

namespace wex
{
/// This class offers a find bar that allows you to find text
/// on a current grid, listview or stc on a frame.
/// Pressing key up and down browses through values from
/// find_replace_data, and pressing enter sets value
/// in find_replace_data.
class find_bar : public ex_commandline
{
public:
  /// Constructor. Fills the bar with value
  /// from find_replace_data.
  find_bar(wex::frame* frame, const data::window& data);

  // Virtual interface

  bool find(bool user_input = true) override;

  bool find_on_enter() override;
};
}; // namespace wex
