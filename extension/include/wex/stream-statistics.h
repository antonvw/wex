////////////////////////////////////////////////////////////////////////////////
// Name:      stream-statistics.h
// Purpose:   Declaration of wex::stream_statistics class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/statistics.h>

namespace wex
{
  class stream;

  /// Offers stream_statistics.
  /// Used by stream to keep statistics.
  class stream_statistics
  {
    friend class stream;
  public:
    /// Adds other statistics.
    stream_statistics& operator+=(const stream_statistics& s) {
      m_Elements += s.m_Elements;
      return *this;}

    /// Returns all items as a string. All items are returned as a string,
    /// with newlines separating items.
    const std::string get() const {return m_Elements.get();};

    /// Returns the key, if not present 0 is returned.
    int get(const std::string& key) const {
      const auto it = m_Elements.get_items().find(key);
      return (it != m_Elements.get_items().end() ? it->second: 0);};

    /// Returns the elements.
    const auto & get_elements() const {return m_Elements;};
  private:
    statistics<int> m_Elements;
  };
};