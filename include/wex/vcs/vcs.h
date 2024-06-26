////////////////////////////////////////////////////////////////////////////////
// Name:      vcs.h
// Purpose:   Declaration of wex::vcs class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <set>
#include <wex/factory/vcs.h>
#include <wex/vcs/vcs-entry.h>

namespace wex
{
class item_dialog;
class path;

namespace factory
{
class frame;
};

/// This class collects all vcs handling.
/// The VCS entries are loaded from menus.xml, this is done
/// during app startup.
class vcs : public factory::vcs
{
public:
  /// The store is a vector of vcs entries.
  typedef std::vector<vcs_entry> store_t;

  // Static interface.

  /// Destroys dialog.
  static void destroy_dialog();

  /// Returns true if specified filename (a path) is a vcs directory.
  static bool dir_exists(const path& filename);

  /// Returns true if vcs is empty.
  static bool empty();

  /// Loads all entries (first clears them) from vcs document.
  /// Returns true if document is loaded.
  static bool load_document();

  /// Exits vcs, cleans up store.
  /// The wex::del::app::OnExit takes care of this.
  static void on_exit();

  /// Initializes the vcs store and loads document.
  /// The wex::del::app::OnInit takes care of this.
  static void on_init();

  /// Returns the number of vcs entries.
  static size_t size();

  // Other methods.

  /// Default constructor.
  vcs(
    /// Specify several files for which you want vcs action.
    /// Sets the vcs entry for first of the specified files, or
    /// to the base folder if vector is empty.
    const std::vector<wex::path>& files = std::vector<wex::path>(),
    /// The command no that is used to set the current vcs command
    /// (index in vcs entry commands).
    int command_no = -1);

  /// Shows a dialog allowing you to choose which vcs to use
  /// and to set the path for each vcs entry.
  /// Returns dialog return code.
  int config_dialog(const data::window& data = data::window()) const;

  /// Returns the current writeable vcs entry.
  auto& entry() { return *m_entry; }

  /// Returns the current readonly vcs entry.
  const auto& entry() const { return *m_entry; }

  /// Executes the current vcs command for the current
  /// vcs entry, and collects the output.
  /// Returns return code from vcs entry execute.
  bool execute();

  /// Executes the specified command (git, svn subcommand).
  /// Return value is false if process could not execute.
  bool execute(const std::string& command);

  /// Returns branch for current vcs entry, or empty string
  /// if vcs is not used.
  const std::string get_branch() const;

  /// Returns name for current vcs entry, or empty string
  /// if vcs is not used.
  const std::string name() const;

  /// Combines show_dialog, execute and vcs entry show_output in one method.
  /// - Returns wxID_CANCEL if dialog was cancelled, or an execute error
  /// occurred.
  /// - Returns wxID_OK if okay (use vcs entry error
  ///   to check whether the output contains errors or normal info).
  wxStandardID request(const data::window& data = data::window());

  /// Sets the vector of paths to specified path.
  void set(const wex::path& p);

  /// Sets the vcs entry using base folder.
  /// If not, it will show
  /// a dialog for selecting a vcs folder (if parent is not nullptr).
  /// Returns true if entry is under vcs control.
  bool set_entry_from_base(
    /// Parent window for showing dir dialog if
    /// there is not a current directory.
    wxWindow* parent = nullptr);

  /// Shows dialog for the current vcs entry.
  int show_dialog(const data::window& data = data::window());

  /// Returns toplevel dir.
  path toplevel() const;

  /// Returns true if vcs usage is set in the config.
  bool use() const;

  // Overridden methods.

  bool is_dir_excluded(const path& p) const override;
  bool is_file_excluded(const path& p) const override;
  bool setup_exclude(const path& dir) override;

private:
  const wex::path current_path() const;

  store_t::iterator m_entry;

  std::vector<wex::path> m_files;
  std::set<wex::path>    m_excludes;
  std::string            m_title;

  static inline item_dialog* m_item_dialog{nullptr};
  static inline store_t*     m_store{nullptr};
};

/// Executes VCS command id for specified files
/// and opens component if necessary.
bool vcs_execute(
  /// frame on which open_file is called
  factory::frame* frame,
  /// VCS menu id to execute
  int id,
  /// files on which to operate
  const std::vector<path>& files,
  /// the window data
  const data::window& data = data::window());
}; // namespace wex
