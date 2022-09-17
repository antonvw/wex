////////////////////////////////////////////////////////////////////////////////
// Name:      global-env.h
// Purpose:   Declaration of class wex::global_env
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015-2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

namespace wex
{
class addressrange;
class ex;
class block_lines;

/// This class offers a class to do global commands on a addressrange.
class global_env
{
public:
  /// Constructor.
  explicit global_env(const addressrange* ar);

  /// Runs the global commands using specified data.
  bool global(const data::substitute& data);

  // Returns true if commands were found.
  bool has_commands() const { return !m_commands.empty(); }

  /// Returns number of hits.
  auto hits() const { return m_hits; }

private:
  bool for_each(const block_lines& match) const;
  bool process(const block_lines& block);
  bool process_inverse(const block_lines& block, block_lines& inverse);
  bool run(const block_lines& block, const std::string& command) const;

  const addressrange* m_ar;

  std::vector<std::string> m_commands;

  int          m_hits{0};
  bool         m_recursive{false};
  ex*          m_ex;
  syntax::stc* m_stc;
};
}; // namespace wex
