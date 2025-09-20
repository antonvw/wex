////////////////////////////////////////////////////////////////////////////////
// Name:      frame.h
// Purpose:   Include file for wex::del::frame class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2009-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/core/config.h>
#include <wex/core/function-repeat.h>
#include <wex/del/defs.h>
#include <wex/del/listview.h>
#include <wex/syntax/indicator.h>
#include <wex/syntax/marker.h>
#include <wex/ui/file-history.h>
#include <wex/ui/frame.h>
#include <wex/ui/item.h>
#include <wex/vcs/vcs.h>

#include <set>

namespace wex
{
class debug;
class item_dialog;
class process;
class stc_entry_dialog;
}; // namespace wex

namespace wex::del
{
class file;

/// Adds file and project history support to frame.
/// It also sets a change indicator in the title of the frame if applicable.
/// Finally it adds find in files and selection dialogs.
class frame : public wex::frame
{
public:
  /// Default constructor.
  /// Default it gives file history support to be used from the file menu.
  /// So you should call use_file_history_list somewhere to set it up.
  /// Default it does not use a recent project file.
  frame(
    size_t              maxFiles    = 9,
    size_t              maxProjects = 0,
    const data::window& data = data::window().style(wxDEFAULT_FRAME_STYLE));

  /// Destructor.
  ~frame() = default;

  // Virtual interface

  /// This method is called to activate a certain listview.
  /// Default it returns nullptr.
  virtual listview*
  activate(wex::data::listview::type_t, const lexer* lexer = nullptr)
  {
    return nullptr;
  }

  /// If there is a project somewhere,
  /// your implementation should return that one.
  /// Default it returns nullptr.
  virtual file* get_project() { return nullptr; }

  // Other methods

  /// Returns a list with default file extensions.
  config::strings_t default_extensions() const;

  /// Finds (or replaces) in specified files.
  /// Returns true if process started.
  bool find_in_files(
    /// the files
    const std::vector<path>& files,
    /// ID_TOOL_REPORT_FIND or ID_TOOL_REPLACE
    const tool& tool,
    /// Default shows a dialog.
    bool show_dialog = true,
    /// report for output
    listview* report = nullptr);

  /// Shows a modal find (or replace) in files dialog.
  /// Returns result from ShowModal.
  int find_in_files_dialog(
    /// ID_TOOL_REPORT_FIND or ID_TOOL_REPLACE
    const tool& tool,
    /// add file types selection as well
    bool add_in_files = false);

  /// Returns caption for find_in_files_dialog.
  const std::string find_in_files_title(wex::window_id id) const;

  /// Debugging interface.
  auto* get_debug() { return m_debug; }

  /// Returns project history.
  auto& get_project_history() { return m_project_history; }

  /// greps for text.
  /// The base directory is the directory for the current stc
  /// component, if available.
  /// Returns true if process started.
  bool grep(
    /// text [extension] [folder]
    const std::string& line,
    /// normally grep does not replace, by setting sed, it can
    bool sed = false);

  /// Opens file from action with a possible extension to move.
  /// If file not empty returns false if an error occurred or no files opened,
  /// and true if at least one file was opened (does not need to exist).
  /// Otherwise always returns true.
  bool open_from_action(
    /// the file, might contain wildcards (* or ?), if empty a dialog is shown
    const std::string& file,
    /// possible extension
    const std::string& move_ext);

  /// Sed (replace in files).
  /// The base directory is the directory for the current stc
  /// component, if available.
  /// Returns true if process started.
  bool sed(
    /// text replacement [extension] [folder]
    const std::string& line)
  {
    return grep(line, true);
  };

  /// Updates project history.
  void set_recent_project(const path& path) { m_project_history.append(path); }

  /// Shows vcs info on statusbar.
  void statustext_vcs(factory::stc* stc);

  /// Starts or stops syncing.
  /// Default syncing is started during construction.
  void sync(bool start = true) { m_function_repeat.activate(start); };

  /// Uses specified history list, and adds all elements from file history
  /// to the list.
  void use_file_history_list(listview* list);

  /// Returns the vcs.
  wex::vcs* vcs() { return m_vcs; };

  /// Shows blame info for vcs in the text margin.
  /// Returns true if info was added.
  bool vcs_blame_show(vcs_entry* vcs, syntax::stc*);

  /// If vcs dialog was open, destroy it;
  void vcs_destroy_dialog();

  // Overridden methods.

  static inline const int id_find_in_files    = ID_FREE_LOWEST;
  static inline const int id_replace_in_files = ID_FREE_LOWEST + 1;

  void bind_accelerators(
    wxWindow*                              parent,
    const std::vector<wxAcceleratorEntry>& v,
    bool                                   debug = false) override;
  void          debug_add_menu(menu& m, bool b) override;
  void          debug_exe(int id, syntax::stc* stc) override;
  void          debug_exe(const std::string& exe, syntax::stc* stc) override;
  wxEvtHandler* debug_handler() override;
  bool          debug_is_active() const override;
  bool          debug_print(const std::string& text) override;
  bool          debug_toggle_breakpoint(int line, syntax::stc* stc) override;

  void on_command_item_dialog(wxWindowID dialogid, const wxCommandEvent& event)
    override;
  void on_notebook(wxWindowID id, wxWindow* page) override;

  bool process_async_system(const process_data& data) override;

  void set_recent_file(const path& path) override;

  void show_ex_bar(int action = HIDE_BAR_FOCUS_STC, syntax::stc* stc = nullptr)
    override;
  void show_ex_message(const std::string& text) override;
  void statusbar_clicked(const std::string&) override;
  void statusbar_clicked_right(const std::string&) override;

  syntax::stc* stc_entry_dialog_component() override;
  int          stc_entry_dialog_show(bool modal = false) override;
  std::string  stc_entry_dialog_title() const override;
  void         stc_entry_dialog_title(const std::string& title) override;
  void         stc_entry_dialog_validator(const std::string& regex) override;

  void vcs_add_path(factory::link*) override;
  bool vcs_annotate_commit(syntax::stc*, int line, const std::string& commit_id)
    override;
  std::string
  vcs_annotate_line(factory::stc* s, const std::string& pane) const override;
  void vcs_append(menu*, const menu_item* i) const override;
  bool vcs_blame(syntax::stc*) override;
  bool vcs_blame_revision(
    syntax::stc*,
    const std::string& renamed,
    const std::string& offset) override;
  bool vcs_dir_exists(const path& p) const override;
  bool vcs_execute(
    int                           event_id,
    const std::vector<wex::path>& paths,
    const data::window&           arg = data::window()) override;
  bool vcs_execute(
    const std::string&            command,
    const std::vector<wex::path>& paths,
    const data::window&           arg = data::window()) override;

  bool vcs_unified_diff(const vcs_entry* e, const unified_diff* uni) override;

  bool vi_is_address(syntax::stc* stc, const std::string& text) const override;

protected:
  /// Access to file history list,
  /// if you use this as a page in a notebook,
  /// you might want prevent closing it.
  auto* file_history_list() { return m_file_history_listview; }

private:
  listview* activate_and_clear(const wex::tool& tool);
  void      bind_all();

  stc_entry_dialog* entry_dialog(
    const std::string& title = std::string(),
    const std::string& text  = std::string());

  void find_in_files(wex::window_id id);

  item_dialog *     m_fif_dialog{nullptr}, *m_rif_dialog{nullptr};
  stc_entry_dialog* m_entry_dialog{nullptr};
  debug*            m_debug{nullptr};
  process*          m_process{nullptr};
  listview*         m_file_history_listview{nullptr};
  class vcs*        m_vcs{nullptr};

  class file_history m_project_history;

  function_repeat m_function_repeat;

  const indicator m_indicator_add = wex::indicator(3);

  const std::string m_text_hidden{_("fif.Hidden")},
    m_text_in_files{_("fif.In files")}, m_text_in_folder{_("fif.In folder")},
    m_text_recursive{_("fif.Recursive")};

  // This set determines what fields are placed on the find_in_files dialogs
  // as a list of checkboxes.
  const item::choices_bool_t m_info;
};

const std::string find_replace_string(bool replace);
}; // namespace wex::del
