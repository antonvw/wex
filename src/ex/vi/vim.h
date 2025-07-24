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
  /// Constructor.
  vim(
    /// vi component
    vi* vi,
    /// vim command to be executed
    std::string& command)

  /// Returns true if this is a vim motion command.
  bool is_motion() const;

  /// Returns true if this is a vim other command.
  bool is_other() const;

  /// Returns true if this command relates to a vim command.
  bool is_vim() const;

  /// Handles a motion command.
  bool motion(
    /// start_pos used to start selecting text
    int start_pos,
    /// indicates how many bytes parsed from executing the vi motion function
    size_t& parsed,
    /// vi motion to execute before excuting vim motion
    const vi::function_t& f);

  /// Handles a other command.
  bool other();

private:
  void command_find(const std::string& command);
  bool command_motion(int pos_start);
  bool command_other();

  vi*          m_vi;
  syntax::stc* m_stc;

  std::string& m_command;

  const std::string    m_command_org;
  const vi::commands_t m_motion_commands, m_other_commands;
};
}; // namespace wex
