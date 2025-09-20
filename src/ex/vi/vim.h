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
    std::string& command);

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

  /// Prepares the motion commands.
  void motion_prep();

  /// Handles a other command.
  bool other();

private:
  /// commands to be used in lambda, see also vi::commands_t
  /// (has a return type in function)
  typedef std::vector<std::pair<
    /// the command
    const std::string,
    /// by this command
    std::function<void(const std::string& command)>>>
    commands_t;

  void command_find(const std::string& command);
  bool command_motion(int pos_start);
  bool command_other();
  void command_z_fold(const std::string& command);

  vi*          m_vi;
  syntax::stc* m_stc;

  std::string& m_command;

  const std::string m_command_org;
  const commands_t  m_motion_commands, m_other_commands;
};
}; // namespace wex
