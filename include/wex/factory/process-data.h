////////////////////////////////////////////////////////////////////////////////
// Name:      factory/process-data.h
// Purpose:   Declaration of class wex::process_data
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <vector>

namespace wex
{
/// This class offers data for process.
class process_data
{
public:
  /// Default constructor, sets the exe (and possible
  /// options).
  process_data(const std::string& exe = std::string());

  /// Returns args as a vector of strings,
  /// or empty vector if no args were provided to the exe.
  const std::vector<std::string> args() const;

  /// Returns exe.
  const auto& exe() const { return m_exe; }

  /// Sets exe.
  process_data& exe(const std::string& rhs);

  /// Returns exe component as a possible path.
  /// It searches the search path for the exe, and
  /// adds the path as prefix if found.
  const std::string exe_path() const;

  /// Logs info.
  const std::string log() const;

  /// Returns start_dir.
  const auto& start_dir() const { return m_start_dir; }

  /// Sets start_dir.
  process_data& start_dir(const std::string& rhs);

  /// Returns stdin.
  const auto& std_in() const { return m_stdin; }

  /// Sets stdin.
  process_data& std_in(const std::string& rhs);

private:
  std::string m_exe, m_start_dir, m_stdin;
};
} // namespace wex
