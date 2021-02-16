////////////////////////////////////////////////////////////////////////////////
// Name:      sort.h
// Purpose:   Include file for wex core utility functions
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <bitset>

namespace wex
{
  namespace core
  {
    class stc;
  };

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
      sort_t sort_t = 0,
      /// position of the first character to be replaced
      size_t pos = 0,
      /// number of characters to replace
      /// string::npos indicates all characters until eol
      size_t len = std::string::npos);

    /// Sorts selected text on specified component, returns true if sorted ok.
    bool selection(core::stc* stc);

    /// Sorts specified input, returns string with sorted text.
    const std::string str(
      /// text to sort
      const std::string& input,
      /// eol to split lines
      const std::string& eol);

  private:
    template <typename T> const std::string get_column(T first, T last);
    template <typename T>
    const std::string get_lines(std::vector<std::string>& lines, T it);

    bool selection_block(core::stc* stc);
    bool selection_other(core::stc* stc);

    const sort_t m_sort_t{0};
    size_t       m_len, m_pos;
  };
} // namespace wex
