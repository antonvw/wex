////////////////////////////////////////////////////////////////////////////////
// Name:      statistics.h
// Purpose:   Include file for class wex::statistics
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <algorithm>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace wex
{
/// Offers base statistics. All statistics involve a key value pair,
/// where the key is a string, and the value a template.
/// There are some virtual methods you can use in a base class e.g. to show
/// the statistics on a grid, that is automatically
/// updated whenever statistics change.
template <class T> class statistics
{
public:
  /// Default constructor.
  /// You can specify a vector of values to initialize the statistics.
  explicit statistics(
    const std::vector<std::pair<const std::string, T>>& v = {})
  {
    if (!std::all_of(
          v.begin(),
          v.end(),
          [this](const auto& it)
          {
            set(it.first, it.second);
            return true;
          }))
    {
      // ignore.
      ;
    }
  };

  /// Destructor.
  virtual ~statistics() = default;

  /// Adds other statistics.
  statistics& operator+=(const statistics& s)
  {
    for (const auto& it : s.m_items)
    {
      inc(it.first, it.second);
    }
    return *this;
  }

  /// Clears the items. If you have shown the statistics
  /// the window is updated as well.
  virtual void clear() { m_items.clear(); }

  /// Decrements key with value.
  const T dec(const std::string& key, T dec_value = 1)
  {
    return set(key, get(key) - dec_value);
  };

  /// Returns true if items are empty.
  bool empty() const { return m_items.empty(); }

  /// Returns all items as a string. All items are returned as a string,
  /// with comma's separating items, and a : separating key and value.
  const std::string get() const
  {
    std::string text;
    for (const auto& it : m_items)
    {
      if (!text.empty())
      {
        text += ", ";
      }

      text += it.first + ":" + std::to_string(it.second);
    }
    return text;
  };

  /// Returns value for specified key.
  const T get(const std::string& key) const
  {
    const auto it = m_items.find(key);
    return it != m_items.end() ? it->second : T();
  };

  /// Returns the items.
  const std::map<std::string, T>& get_items() const { return m_items; }

  /// Increments key with value.
  const T inc(const std::string& key, T inc_value = 1)
  {
    return set(key, get(key) + inc_value);
  };

  /// Sets key to value. If you have shown the statistics
  /// the window is updated as well.
  virtual const T set(const std::string& key, T value)
  {
    m_items[key] = value;
    return value;
  };

private:
  std::map<std::string, T> m_items;
};
}; // namespace wex
