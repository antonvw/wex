////////////////////////////////////////////////////////////////////////////////
// Name:      core.h
// Purpose:   Include file for wex core utility functions
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <vector>

namespace wex
{
  /// Regular expression match.
  /// Returns:
  /// - -1 if text does not match or there is an error
  /// - 0 if text matches, but no submatches present, v is untouched
  /// - submatches, it fills v with the submatches
  int match(
    /// regular expression
    const std::string& regex,
    /// text to match
    const std::string& text,
    /// vector is filled with submatches
    std::vector<std::string>& v);
} // namespace wex
