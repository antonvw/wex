////////////////////////////////////////////////////////////////////////////////
// Name:      marker.h
// Purpose:   Declaration of class wex::marker
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/presentation.h>

namespace wex
{
/// This class defines our scintilla markers.
class marker : public presentation
{
public:
  /// Default constructor.
  explicit marker(const pugi::xml_node& node = pugi::xml_node());

  /// Constructor.
  /// Only sets no and symbol, and not the colours.
  explicit marker(int no, int symbol = -1);

  /// Returns symbol no.
  int symbol() const { return style(); }
};
}; // namespace wex
