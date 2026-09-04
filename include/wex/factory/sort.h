////////////////////////////////////////////////////////////////////////////////
// Name:      sort.h
// Purpose:   Declaration of wex::sort class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <bitset>

namespace wex
{
namespace factory
{
class stc;
class sort_data;

/// Offers a sort class to sort text blocks, from strings or stc component.
class sort
{
  friend class sort_data;

public:
  /// The sort flags.
  enum
  {
    SORT_DESCENDING = 0, ///< sort descending order
    SORT_UNIQUE     = 1, ///< flag to remove doubles
  };

  /// A typedef containing sort flags.
  using sort_t = std::bitset<2>;

  /// Default constructor.
  sort(
    /// sort type
    sort_t t = sort_t(),
    /// position of the first character to be sorted
    size_t pos = 0,
    /// number of characters to sort
    /// string::npos indicates all characters until eol
    size_t len = std::string::npos);

  /// Sorts selected text on specified component, returns true if sorted ok.
  bool selection(factory::stc* stc) const;

  /// Sorts specified input, returns string with sorted text.
  const std::string string(
    /// text to sort
    const std::string& input,
    /// characters to tokenize the input
    const std::string& separators) const;

private:
  bool selection_block(factory::stc* stc) const;
  bool selection_other(factory::stc* stc) const;

  const sort_t m_sort_t;
  const size_t m_len, m_pos;
};
} // namespace factory
} // namespace wex
