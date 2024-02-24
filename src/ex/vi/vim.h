////////////////////////////////////////////////////////////////////////////////
// Name:      vim.h
// Purpose:   Declaration of wex::vim class to handle vim special commands
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "motion.h"

namespace wex
{
enum class motion_t;

/// This class offers vim commands handling (g commands).
class vim
{
public:
  /// Returns the motion type for specified command.
  static vi::motion_t get_motion(const std::string& command);

  /// Constructor.
  vim(vi* vi, std::string& command, vi::motion_t t);

  /// Returns true if this is a vim motion command.
  bool is_motion() const;

  /// Returns true if this is a vim special command.
  bool is_special() const;

  /// Handles the motion commands.
  bool motion(int start_pos, size_t& parsed, const vi::function_t& t);

  /// Prepares the motion commands.
  void motion_prep();

  /// Handles the special commands.
  bool special();

private:
  bool command_motion(int pos_start);
  bool command_special();

  vi* m_vi;

  std::string&       m_command;
  const vi::motion_t m_motion;
};
}; // namespace wex
