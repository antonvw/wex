////////////////////////////////////////////////////////////////////////////////
// Name:      util.h
// Purpose:   ex utility methods
// Author:    Anton van Wezenbeek
// Copyright: (c) 2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <algorithm>

namespace wex
{
// Finds a command out of commands.
/// Returns the iterator to the found command, or end iterator.
template <typename T>
T::const_iterator find_from(
  /// the commands, should be a container of pairs with string
  /// with command chars and e.g. a callback
  const T& commands,
  /// the command chars, finds the first char that matches
  const std::string& chars)
{
  if (commands.empty() || chars.empty())
  {
    return commands.end();
  }

  const auto& it = std::ranges::find_if(
    commands,
    [&](auto const& e)
    {
      return std::ranges::any_of(
        e.first,
        [chars](const auto& p)
        {
          return p == chars[0];
        });
    });

  return it;
};

/// Returns true if a register is specified by the text (normal or calc).
bool is_register_valid(const std::string& text);
}; // namespace wex
