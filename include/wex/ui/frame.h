////////////////////////////////////////////////////////////////////////////////
// Name:      frame.h
// Purpose:   Declaration of wex::frame class.
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/core/path.h>
#include <wex/data/stc.h>
#include <wex/factory/frame.h>
#include <wex/factory/window.h>
#include <wex/ui/file-history.h>
#include <wex/ui/statusbar-pane.h>
#include <wex/ui/statusbar.h>
#include <wx/aui/framemanager.h>

#include <utility>
#include <vector>

class wxFindReplaceDialog;
class wxPanel;

namespace wex
{
class debug_entry;
class ex_command;
class ex_commandline;
class line_data;
class menu_item;
class process_data;
class toolbar;

namespace factory
{
class link;
};

namespace syntax
{
class stc;
};

/// Offers an aui managed frame with a notebook multiple document interface,
/// used by the notebook classes, and toolbar, findbar and vibar support.
/// - The toolbar and findbar are added as toolbarpanes to the aui manager.
/// - The vibar is added as normal aui panel to the aui manager.
/// The next panes are supported:
/// - FINDBAR
/// - OPTIONSBAR
/// - PROCESS
/// - TOOLBAR
/// - VIBAR (same as the ex bar)
class frame : public factory::frame
{
public:
  /// Enums for show_ex_bar.
  enum
  {
    HIDE_BAR,                 ///< hide bar, unless there is no statusbar
    HIDE_BAR_FOCUS_STC,       ///< as previous, and focus to stc
    HIDE_BAR_FORCE,           ///< hide bar, even if there is no statusbar
    HIDE_BAR_FORCE_FOCUS_STC, ///< as previous, and focus to stc
    SHOW_BAR,                 ///< shows bar
    SHOW_BAR_SYNC_CLOSE_ALL,  ///< shows bar, but all components are closed
  };

  /// Panes vector with a pair of panes
  typedef std::vector<std::pair<wxWindow*, wxAuiPaneInfo>> panes_t;

  /// Toggled panes type.
  typedef std::vector<std::pair<std::pair<std::string, std::string>, int>>
    toggled_panes_t;

  /// Default constructor, registers the aui manager, and creates the bars.
  frame(
    size_t              maxFiles = 9,
    const data::window& data     = data::window().style(wxDEFAULT_FRAME_STYLE));

  /// Destructor, uninits the aui manager.
  ~frame() override;

  // Virtual interface

  /// Returns true if the page can be closed.
  /// Default resets the find focus.
  virtual bool allow_close(
    /// notebook id
    wxWindowID id,
    /// page
    wxWindow* page);

  /// Appends vcs menu.
  virtual void append_vcs(menu*, const menu_item* i) const { ; }

  /// Binds accelerators.
  virtual void bind_accelerators(
    wxWindow*                              parent,
    const std::vector<wxAcceleratorEntry>& v,
    bool                                   debug = false)
  {
    ;
  };

  /// Adds debug menu.
  virtual void debug_add_menu(menu&, bool) { ; }

  /// Runs debug action.
  virtual void debug_exe(int menu_id, syntax::stc* stc) { ; }

  /// Runs debug command.
  virtual void debug_exe(const std::string& command, syntax::stc* stc) { ; }

  /// Invoked when a debug exe path is opened.
  virtual void debug_exe(const path& p) { ; }

  /// Sets a debug handler.
  virtual wxEvtHandler* debug_handler() { return nullptr; }

  /// Runs true if we are debugging.
  virtual bool debug_is_active() const { return false; }

  /// Prints a debug variable.
  virtual bool debug_print(const std::string& text) { return false; }

  /// Toggles a breakpoint on line.
  virtual bool debug_toggle_breakpoint(int line, syntax::stc* stc)
  {
    return false;
  };

  /// Executes a ex command. Returns true if
  /// this command is handled. This method is invoked
  /// at the beginning of the ex command handling,
  /// allowing you to override any command.
  virtual bool exec_ex_command(ex_command& command) { return false; }

  /// Allows you to override is_address, it is overridden in
  /// del frame, returning true if text specifies
  /// a one or two address based ex address (including command).
  /// e.g. 1,5y, %y, etc.
  virtual bool is_address(syntax::stc* stc, const std::string& text)
  {
    return false;
  };

  /// Moves to next page. If none or only one page present, returns false.
  virtual bool next_page() { return false; }

  /// Called if the notebook changed page.
  virtual void on_notebook(wxWindowID id, wxWindow* page) { ; }

  /// Called on browse forward, backward.
  virtual void open_file_same_page(const wex::path& p) { ; }

  /// Allows you to override print ex.
  virtual bool print_ex(syntax::stc* stc, const std::string& text)
  {
    return false;
  };

  /// Allows you to perform action for a (vi) command.
  /// This method is invoked after command is executed.
  virtual void record(const std::string& command);

  /// Restores a previous saved current page.
  /// Returns restored page (default returns nullptr).
  virtual syntax::stc* restore_page(const std::string& key) { return nullptr; };

  /// Saves the current page, to restore later on.
  virtual bool save_current_page(const std::string& key) { return false; }

  /// Act on a double shift click.
  virtual void shift_double_click() { ; }

  /// Shows or hides the ex bar.
  /// Default it hides the ex bar and
  /// sets focus back to stc component associated with current ex.
  /// If current ex has ex mode enabled, the ex bar is always shown.
  virtual void show_ex_bar(
    /// action
    int action = HIDE_BAR_FOCUS_STC,
    /// stc component to use for showing ex bar (for SHOW_ actions)
    syntax::stc* stc = nullptr)
  {
    ;
  };

  /// Shows text in ex bar.
  virtual void show_ex_message(const std::string& text) { ; }

  /// Shows or updates stc entry dialog.
  virtual int show_stc_entry_dialog(bool modal = false) { return wxID_CANCEL; }

  /// Returns stc component for stc entry dialog.
  virtual syntax::stc* stc_entry_dialog_component() { return nullptr; }

  /// Returns stc entry dialog title.
  virtual std::string stc_entry_dialog_title() const { return std::string(); }

  /// Sets stc entry dialog title.
  virtual void stc_entry_dialog_title(const std::string& title) { ; }

  /// Sets stc entry dialog validator.
  virtual void stc_entry_dialog_validator(const std::string& regex) { ; }

  /// Called after you checked the Sync checkbox on the options toolbar.
  virtual void sync_all() { ; }

  /// Called after all pages from the notebooks are deleted.
  /// Default resets the find focus.
  virtual void sync_close_all(wxWindowID id);

  /// Adds vcs path.
  virtual void vcs_add_path(factory::link* l) { ; }

  /// Annotates commit.
  virtual bool
  vcs_annotate_commit(syntax::stc*, int line, const std::string& commit_id)
  {
    return false;
  };

  /// Blames the specified stc.
  virtual bool vcs_blame(syntax::stc*) { return false; }

  /// Blames revision.
  virtual bool vcs_blame_revision(
    syntax::stc*,
    const std::string& renamed,
    const std::string& offset)
  {
    return false;
  }

  /// Returns true if dir exists.
  virtual bool vcs_dir_exists(const path& p) const { return false; };

  /// Executes vcs.
  virtual bool vcs_execute(
    /// the vcs id
    int event_id,
    /// the paths
    const std::vector<wex::path>& paths,
    /// window data
    const data::window& arg = data::window())
  {
    return false;
  }

  // Other methods

  /// Can we browse backward?
  bool allow_browse_backward() const;

  /// Can we browse forward?
  bool allow_browse_forward() const;

  /// Handles forward and backward click.
  bool browse(const wxCommandEvent& event);

  /// Returns current debugger.
  const auto* debug_entry() const { return m_debug_entry; }

  /// Returns file history.
  auto& file_history() { return m_file_history; }

  /// Returns the find toolbar.
  auto* get_find_toolbar() { return m_findbar; }

  /// Returns the options toolbar.
  auto* get_options_toolbar() { return m_optionsbar; }

  /// Returns statusbar.
  auto* get_statusbar() { return m_statusbar; }

  /// Returns the toolbar.
  auto* get_toolbar() { return m_toolbar; }

  /// Adds a window as a pane, generating a unique name.
  /// Return the name of the pane.
  const std::string pane_add(wxWindow* pane);

  /// Add panes to the manager.
  /// Returns false if one of the panes could not be added.
  bool pane_add(
    /// panes
    const panes_t& panes,
    /// name of perspective to load / save
    const std::string& perspective = "managed frame");

  /// Returns window related to window, or nullptr if pane
  /// is not present.
  wxWindow* pane_get(const std::string& pane);

  /// Returns true if the managed pane is maximized.
  bool pane_is_maximized(const std::string& pane)
  {
    return m_manager.GetPane(pane).IsMaximized();
  }

  /// Returns true if pane is shown.
  bool pane_is_shown(const std::string& pane)
  {
    return m_manager.GetPane(pane).IsShown();
  }

  /// Maximizes the managed pane.
  /// Returns false if pane is not managed.
  bool pane_maximize(const std::string& pane);

  /// Restores the managed pane.
  /// Returns false if pane is not managed.
  bool pane_restore(const std::string& pane);

  /// Updates pane info for managed pane.
  /// Returns false if pane is not managed.
  bool pane_set(const std::string& pane, const wxAuiPaneInfo& info);

  /// Shows or hides the managed pane.
  /// Returns false if pane is not managed.
  bool pane_show(const std::string& pane, bool show = true);

  /// Toggles the managed pane: if shown hides it, otherwise shows it.
  /// Returns false if pane is not managed.
  bool pane_toggle(const std::string& pane)
  {
    return pane_show(pane, !pane_is_shown(pane));
  };

  /// Returns number of panes.
  size_t panes() const;

  /// Sets debug entry.
  void set_debug_entry(wex::debug_entry* de) { m_debug_entry = de; }

  /// Sets up the status bar if you want to use statustext.
  /// And initializes other static data.
  statusbar* setup_statusbar(
    const std::vector<statusbar_pane>& panes,
    long                               style = wxST_SIZEGRIP,
    const std::string&                 name  = "statusBar");

  /// Returns a command line ex command.
  /// Shows the ex bar, sets the label and sets focus to it, allowing
  /// you to enter a command.
  /// Returns false if label is not supported.
  bool show_ex_command(
    /// the ex on which command is to be done
    syntax::stc* stc,
    /// label for the ex bar (/, ?, :, =)
    const std::string& label);

  /// Shows ex bar, and enters input mode.
  /// Returns false if command is not supported.
  bool show_ex_input(
    /// the ex on which command is to be done
    syntax::stc* stc,
    /// the command (a, c, or i)
    char command);

  /// Shows or hide process pane.
  void show_process(bool show);

  /// Returns the toggled panes.
  const auto& toggled_panes() const { return m_toggled_panes; }

  /// overridden methods

  std::string get_statustext(const std::string& pane) const override;
  factory::stc*
  open_file(const path& filename, const data::stc& data = data::stc()) override;
  bool output(const std::string& text) const override;
  void set_recent_file(const path& path) override;
  void statusbar_clicked_right(const std::string&) override;
  bool
  statustext(const std::string& text, const std::string& pane) const override;

  bool Destroy() override;
  bool Show(bool show = true) override;
  wxStatusBar*
  OnCreateStatusBar(int number, long style, wxWindowID id, const wxString& name)
    override;

protected:
  void on_menu_history(
    const class file_history& history,
    size_t                    index,
    data::stc::window_t       flags = 0);

  class debug_entry* m_debug_entry{nullptr};
  statusbar*         m_statusbar{nullptr};
  ex_commandline*    m_ex_commandline;

private:
  bool     add_toolbar_panes(const panes_t& panes);
  wxPanel* create_ex_panel();
  void     provide_output(const std::string& text) const;

  std::string m_perspective;

  std::ofstream* m_ofs{nullptr};

  const toggled_panes_t m_toggled_panes;

  wxFindReplaceDialog* m_find_replace_dialog{nullptr};

  toolbar *m_findbar, *m_optionsbar, *m_toolbar;

  wxAuiManager       m_manager;
  class file_history m_file_history;

  size_t m_browse_index{0};
};
}; // namespace wex
