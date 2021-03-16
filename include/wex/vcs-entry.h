////////////////////////////////////////////////////////////////////////////////
// Name:      vcs-entry.h
// Purpose:   Declaration of wex::vcs_entry class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <pugixml.hpp>
#include <wex/blame.h>
#include <wex/lexer.h>
#include <wex/menu-commands.h>
#include <wex/process.h>
#include <wex/vcs-command.h>

namespace wex
{
  class menu;

  /// This class collects a single vcs.
  class vcs_entry
    : public process
    , public menu_commands<vcs_command>
  {
  public:
    enum
    {
      FLAGS_LOCATION_POSTFIX,
      FLAGS_LOCATION_PREFIX
    };

    /// Default constructor using xml node.
    vcs_entry(const pugi::xml_node& node = pugi::xml_node());

    /// Returns the administrative directory.
    const auto& admin_dir() const { return m_admin_dir; };

    /// Builds a menu from all vcs commands.
    /// Returns (total) number of items in menu.
    size_t build_menu(
      /// menu id to be added to the vcs commands
      int base_id,
      /// menu to be built
      menu* menu) const;

    /// Executes the current vcs command (from SetCommand), or
    /// the first command if SetCommand was not yet invoked.
    /// Might ask for vcs binary if it is not yet known.
    /// Return code is code from process Execute,
    /// and also can be false if dialog for vcs bin was cancelled.
    bool execute(
      /// args, like filenames, or vcs flags
      const std::string& args = std::string(),
      /// lexer that is used for presenting the output
      const lexer& lexer = wex::lexer(),
      /// working directory
      const std::string& wd = std::string());

    /// Executes the command.
    /// Return value is false if process could not execute.
    bool execute(
      /// command to be executed
      const std::string& command,
      /// working dir
      const std::string& wd);

    /// Returns flags location.
    auto flags_location() const { return m_flags_location; };

    /// Returns blame info.
    const blame& get_blame() const { return m_blame; };

    /// Returns the name of current branch.
    const std::string get_branch(const std::string& wd = std::string()) const;

    /// Returns the flags used to run the command.
    const std::string get_flags() const;

    /// Executes the log command using the log id to be retrieved.
    /// Return value is false if process could not execute.
    bool log(
      /// path to be used
      const path& p,
      /// id to be retrieved
      const std::string& id);

    /// Virtual interface

    void show_output(const std::string& caption = std::string()) const override;

  private:
    const std::string bin() const;

    // no const, as entry is set using operator+ in vcs.
    int m_flags_location{FLAGS_LOCATION_POSTFIX};

    std::string m_admin_dir, m_log_flags;

    class blame m_blame;
    lexer       m_lexer;
  };
}; // namespace wex
