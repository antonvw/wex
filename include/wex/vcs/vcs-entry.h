////////////////////////////////////////////////////////////////////////////////
// Name:      vcs-entry.h
// Purpose:   Declaration of wex::vcs_entry class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2010-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <optional>
#include <pugixml.hpp>
#include <set>
#include <wex/core/vcs-command.h>
#include <wex/syntax/blame.h>
#include <wex/syntax/lexer.h>
#include <wex/ui/menu-commands.h>
#include <wex/vcs/process.h>

namespace wex
{
class item_dialog;
class menu;
class process_data;

/// This class collects a single vcs.
class vcs_entry
  : public process
  , public menu_commands<vcs_command>
{
public:
  /// Flags location flags, indicate where flags from xml
  /// should be placed when building vcs commands.
  enum class flags_location_t
  {
    POSTFIX, /// after e.g. id
    PREFIX   /// before e.g. id
  };

  /// Default constructor using xml node.
  vcs_entry(const pugi::xml_node& node = pugi::xml_node());

  // Virtual overrides.

  /// Executes the command synchronously.
  /// You just need to specify the flags, the binary itself is
  /// prefixed by wex. Flags available from git menu commands xml
  /// are also added.
  /// Return value is the process exit code.
  int system(const process_data& data) override;

  // Other methods.

  /// Returns the administrative directory.
  const auto& admin_dir() const { return m_admin_dir; }

  /// Returns binary.
  const std::string bin() const;

  /// Builds a menu from all vcs commands.
  /// Returns (total) number of items in menu.
  size_t build_menu(
    /// menu id to be added to the vcs commands
    int base_id,
    /// menu to be built
    menu* menu) const;

  /// Executes the current vcs command synchronously (from set_command), or
  /// the first command if set_command was not yet invoked.
  /// Might ask for vcs binary if it is not yet known.
  /// Return code is code from process execute,
  /// and also can be false if dialog for vcs bin was cancelled.
  bool execute(
    /// args, like filenames, or vcs flags
    const std::string& args = std::string(),
    /// lexer that is used for presenting the output
    const path& p = wex::path(),
    /// working directory
    const std::string& wd = std::string());

  /// Returns flags location.
  auto flags_location() const { return m_flags_location; }

  /// Returns blame info.
  const blame& get_blame() const { return m_blame; }

  /// Returns blame info.
  blame& get_blame() { return m_blame; }

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

  /// Sets up the exclude dir.
  /// Returns vector with paths to be excluded.
  std::optional<std::set<wex::path>>
  setup_exclude(const path& toplevel, const path& p);

  // Virtual interface

  void show_output(const std::string& caption = std::string()) const override;

private:
  int revisions_dialog(
    const std::string& path,
    const wex::path&   tl,
    const wex::path&   file);

  // no const, as entry is set using operator= in vcs.
  flags_location_t m_flags_location{flags_location_t::POSTFIX};

  std::string m_admin_dir, m_log_flags;

  item_dialog* m_item_dialog{nullptr};

  class blame m_blame;
  lexer       m_lexer;
};
}; // namespace wex
