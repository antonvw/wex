////////////////////////////////////////////////////////////////////////////////
// Name:      vcs_command.h
// Purpose:   Declaration of wex::vcs_command class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <wex/menucommand.h>

namespace wex
{
  /// This class contains a single vcs command.
  class vcs_command : public menu_command
  {
  public:
    /// Default constructor.
    vcs_command() {;};

    /// Constructor.
    vcs_command(
      /// Specify command.
      const std::string& command,
      /// The type (main, popup, both).
      const std::string& type = std::string(),
      /// The submenu member is set to specified submenu if not empty,
      /// otherwise to specified subcommand.
      const std::string& submenu = std::string(),
      /// The subcommand (used as submenu, but also used for executing).
      const std::string& subcommand = std::string(),
      /// The flags.
      const std::string& flags = std::string())
      : menu_command(command, type, submenu, subcommand, flags) {;};

    /// Returns true if this is a add like command.
    bool is_add() const;
    
    /// Returns true if this is a blame like command.
    bool is_blame() const;

    /// Returns true if this is a checkout like command.
    bool is_checkout() const;

    /// Returns true if this is a commit like command.
    bool is_commit() const;

    /// Returns true if this is a diff like command.
    bool is_diff() const;

    /// Returns true if this is a history like command.
    bool is_history() const;
    
    /// Returns true if this command can behave like
    /// opening a file.
    bool is_open() const;
  };
};
