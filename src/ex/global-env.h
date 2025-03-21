////////////////////////////////////////////////////////////////////////////////
// Name:      global-env.h
// Purpose:   Declaration of class wex::global_env
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

namespace wex
{
class addressrange;
class addressrange_mark;
class ex;
class block_lines;

/// This class offers a class to do global commands on an ex component.
/// All changes can be undone in a single Undo (see addressrange_mark).
class global_env
{
public:
  /// Constructor. Specify addressrange, usually this range is not
  /// specified, and implicit the entire range is assumed.
  /// However you can select a 2addr range.
  /// Next, it uses the addressrange static data commands for
  /// the global command.
  explicit global_env(const addressrange& ar);

  /// Runs the global commands using specified data.
  /// Returns false if an error occurred.
  bool global(const data::substitute& data);

  // Returns true if commands were found.
  bool has_commands() const { return !m_commands.empty(); }

  /// Returns number of hits.
  auto hits() const { return m_hits; }

private:
  bool command(const block_lines& block, const std::string& text) const;
  bool for_each(const block_lines& match) const;
  bool process(const block_lines& block);
  bool process_inverse(
    const addressrange_mark& am,
    const block_lines&       block,
    block_lines&             inverse);

  const addressrange m_ar;

  std::vector<std::string> m_commands;

  int          m_hits{0};
  bool         m_recursive{false};
  ex*          m_ex;
  syntax::stc* m_stc;
};
}; // namespace wex
