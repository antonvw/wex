////////////////////////////////////////////////////////////////////////////////
// Name:      stream-statistics.h
// Purpose:   Declaration of wxExStreamStatistics class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/extension/statistics.h>

class wxExStream;

/// Offers stream statistics.
/// Used by wxExStream to keep statistics.
class WXDLLIMPEXP_BASE wxExStreamStatistics
{
  friend class wxExStream;
public:
  /// Adds other statistics.
  wxExStreamStatistics& operator+=(const wxExStreamStatistics& s) {
    m_Elements += s.m_Elements;
    return *this;}

  /// Returns all items as a string. All items are returned as a string,
  /// with newlines separating items.
  const std::string Get() const {return m_Elements.Get();};

  /// Returns the key, if not present 0 is returned.
  int Get(const std::string& key) const {
    const auto it = m_Elements.GetItems().find(key);
    return (it != m_Elements.GetItems().end() ? it->second: 0);};

  /// Returns the elements.
  const auto & GetElements() const {return m_Elements;};
private:
  wxExStatistics<int> m_Elements;
};
