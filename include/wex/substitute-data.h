////////////////////////////////////////////////////////////////////////////////
// Name:      substitute-data.h
// Purpose:   Declaration of class wex::data::substitute
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>

namespace wex::data
{
  /// Holds substitute data for addressrange.
  class substitute
  {
  public:
    /// Default constructor.
    substitute(
      const std::string& pattern     = std::string(),
      const std::string& replacement = std::string(),
      const std::string& options     = std::string());

    /// Returns whether options indicate confirmed.
    bool is_confirmed() const;

    /// Returns whether options indicate global.
    bool is_global() const;

    /// Returns whether options indicate ignore case.
    bool is_ignore_case() const;

    /// Returns pattern.
    auto& pattern() const { return m_pattern; };

    /// Returns replacement.
    auto& replacement() const { return m_replacement; };

    /// Sets data from text.
    bool
    set(const std::string& text, const std::string& pattern = std::string());

  private:
    std::string m_pattern, m_replacement, m_options;
  };
} // namespace wex::data
