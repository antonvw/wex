////////////////////////////////////////////////////////////////////////////////
// Name:      stream-statistics.h
// Purpose:   Declaration of wex::stream_statistics class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/statistics.h>

namespace wex
{
/// Offers stream_statistics.
/// Used by stream to keep statistics.
class stream_statistics
{
public:
  /// Adds other statistics.
  stream_statistics& operator+=(const stream_statistics& s)
  {
    m_elements += s.m_elements;
    return *this;
  }

  /// Clears the statistics.
  void clear() { m_elements.clear(); }

  /// Returns true if statistics are empty.
  bool empty() const { return m_elements.empty(); }

  /// Returns all items as a string. All items are returned as a string,
  /// with newlines separating items.
  const std::string get() const { return m_elements.get(); }

  /// Returns the key, if not present 0 is returned.
  int get(const std::string& key) const;

  /// Returns the elements.
  const auto& get_elements() const { return m_elements; }

  /// Returns the elements.
  auto& get_elements() { return m_elements; }

  /// Increments keyword.
  int inc(const std::string& keyword, int inc_value = 1);

  /// Increments actions.
  int inc_actions();

  /// Increments actions comleted.
  int inc_actions_completed(int inc_value = 1);

private:
  statistics<int> m_elements;
};

// implementation

inline int wex::stream_statistics::get(const std::string& key) const
{
  const auto it = m_elements.get_items().find(key);
  return (it != m_elements.get_items().end() ? it->second : 0);
}

inline int
wex::stream_statistics::inc(const std::string& keyword, int inc_value)
{
  return m_elements.inc(keyword, inc_value);
}

inline int wex::stream_statistics::inc_actions()
{
  return inc(_("Files").ToStdString());
}

inline int wex::stream_statistics::inc_actions_completed(int inc_value)
{
  return inc(_("Actions Completed").ToStdString(), inc_value);
}
}; // namespace wex
