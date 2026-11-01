////////////////////////////////////////////////////////////////////////////////
// Name:      indicator.h
// Purpose:   Declaration of class wex::indicator
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/syntax/presentation.h>

namespace wex
{
/// This class defines our scintilla indicators.
class indicator : public presentation
{
public:
  /// Indicator type.
  /// This must match indicators specified in wex-lexers.xml.
  enum indicator_t
  {
    IND_MATCH = 0,    ///< line match indicator
    IND_ERROR,        ///< error indicator
    IND_BREAKPOINT,   ///< debug breakpoint indicator
    IND_DIFF_ADD,     ///< difference add indicator
    IND_INACTIVE_CODE ///< inactive code indicator
  };

  /// Default constructor.
  explicit indicator(const pugi::xml_node& node = pugi::xml_node());

  /// Constructor.
  /// Only sets no and style, and not the colour and under.
  explicit indicator(int no, int style = -1);
};
}; // namespace wex
