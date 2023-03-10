////////////////////////////////////////////////////////////////////////////////
// Name:      indicator.h
// Purpose:   Declaration of class wex::indicator
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/syntax/presentation.h>

namespace wex
{
/// This class defines our scintilla indicators.
class indicator : public presentation
{
public:
  /// Default constructor.
  explicit indicator(const pugi::xml_node& node = pugi::xml_node());

  /// Constructor.
  /// Only sets no and style, and not the colour and under.
  explicit indicator(int no, int style = -1);
};
}; // namespace wex
