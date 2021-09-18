////////////////////////////////////////////////////////////////////////////////
// Name:      data/substitute.h
// Purpose:   Declaration of class wex::data::substitute
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

import<string>;

namespace wex::data
{
/// Holds substitute data for addressrange.
class substitute
{
public:
  /// Default constructor, calls set.
  substitute(const std::string& text = std::string());

  /// Returns whether options indicate confirmed.
  bool is_confirmed() const;

  /// Returns whether options indicate global.
  bool is_global() const;

  /// Returns whether options indicate ignore case.
  bool is_ignore_case() const;

  /// Returns commands.
  auto& commands() const { return m_commands; }

  /// Returns pattern.
  auto& pattern() const { return m_pattern; }

  /// Returns replacement.
  auto& replacement() const { return m_replacement; }

  /// Sets pattern, replacement, options from text:
  /// s/pattern/text/options
  bool set(const std::string& text);

  /// Sets pattern, commands from text (for global substitute).
  bool set_global(const std::string& text);

  /// Sets options only.
  void set_options(const std::string& text);

private:
  std::string m_commands, m_options, m_pattern, m_replacement;
};
} // namespace wex::data
