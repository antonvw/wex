////////////////////////////////////////////////////////////////////////////////
// Name:      vim.h
// Purpose:   Declaration of wex::vim class to handle vim commands
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "motion.h"

namespace wex
{
enum class motion_t;

/// This class offers vim commands handling (g or z commands).
class vim
{
public:
  /// Returns the motion type for specified command.
  static vi::motion_t get_motion(const std::string& command);

  /// Constructor.
  vim(vi* vi, std::string& command, vi::motion_t t);

  /// Returns true if this is a vim motion command.
  bool is_motion() const;

  /// Returns true if this is a vim other command.
  bool is_other() const;

  /// Returns true if this command starts with a vim command.
  bool is_vim() const;

  /// Handles the motion commands.
  bool motion(int start_pos, size_t& parsed, const vi::function_t& t);

  /// Prepares the motion commands.
  void motion_prep();

  /// Handles other commands.
  bool other();

private:
  bool command_motion(int pos_start);
  bool command_other();
  void command_z();

  vi*          m_vi;
  syntax::stc* m_stc;

  std::string&       m_command;
  const vi::motion_t m_motion;
};
}; // namespace wex
