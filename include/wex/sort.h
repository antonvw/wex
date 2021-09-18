////////////////////////////////////////////////////////////////////////////////
// Name:      sort.h
// Purpose:   Declaration of wex::sort class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

import<bitset>;

namespace wex
{
namespace factory
{
class stc;
};

/// Offers a sort class to sort text blocks, from strings or stc component.
class sort
{
public:
  enum
  {
    SORT_DESCENDING = 0, ///< sort descending order
    SORT_UNIQUE     = 1, ///< flag to remove doubles
  };

  typedef std::bitset<2> sort_t;

  /// Default constructor.
  sort(
    /// sort type
    sort_t sort_t = 0,
    /// position of the first character to be sorted
    size_t pos = 0,
    /// number of characters to sort
    /// string::npos indicates all characters until eol
    size_t len = std::string::npos);

  /// Sorts selected text on specified component, returns true if sorted ok.
  bool selection(factory::stc* stc);

  /// Sorts specified input, returns string with sorted text.
  const std::string string(
    /// text to sort
    const std::string& input,
    /// characters to split lines
    const std::string& separators);

private:
  template <typename T> const std::string get_column(T first, T last);
  template <typename T>
  const std::string get_lines(std::vector<std::string>& lines, T it);

  bool selection_block(factory::stc* stc);
  bool selection_other(factory::stc* stc);

  const sort_t m_sort_t{0};
  size_t       m_len, m_pos;
};
} // namespace wex
