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

/// This class offers a class to do global commands on a addressrange.
class global_env
{
public:
  /// Constructor.
  explicit global_env(const addressrange* ar);

  /// Destructor.
  ~global_env();

  /// Runs the global command using specified data.
  bool global(const data::substitute& data);

  // Returns true if commands were found.
  bool has_commands() const { return !m_commands.empty(); }

  /// Returns number of hits.
  auto hits() const { return m_hits; }

private:
  /// Do global on specified line.
  bool for_each(int line) const;

  /// Do global on range from start to end line (excluded).
  bool for_each(int start, int& end, int& hits) const;

  const addressrange* m_ar;

  std::vector<std::string> m_commands;

  int           m_changes{0}, m_hits{0};
  ex*           m_ex;
  factory::stc* m_stc;
};
}; // namespace wex
