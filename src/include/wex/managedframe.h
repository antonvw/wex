////////////////////////////////////////////////////////////////////////////////
// Name:      managedframe.h
// Purpose:   Declaration of wex::managed_frame class.
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <utility>
#include <vector>
#include <wex/filehistory.h>
#include <wex/frame.h>
#include <wex/path.h>
#include <wx/aui/framemanager.h> // for wxAuiManager

class wxPanel;

namespace wex
{
  class debug;
  class ex;
  class ex_command;
  class textctrl;
  class toolbar;

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
  class managed_frame : public frame
  {
  public:
    /// Enums for hide_ex_bar.
    enum
    {
      HIDE_BAR,                 ///< hide bar, unless there is no statusbar
      HIDE_BAR_FOCUS_STC,       ///< as previous, and focus to stc
      HIDE_BAR_FORCE,           ///< hide bar, even if there is no statusbar
      HIDE_BAR_FORCE_FOCUS_STC, ///< as previous, and focus to stc
    };

    /// Panes vector with a pair of panes
    typedef std::vector<std::pair<wxWindow*, wxAuiPaneInfo>> panes_t;

    /// Toggled panes type.
    typedef std::vector<std::pair<std::pair<std::string, std::string>, int>>
      toggled_panes_t;

    /// Default constructor, registers the aui manager, and creates the bars.
    managed_frame(
      size_t              maxFiles = 9,
      const data::window& data = data::window().style(wxDEFAULT_FRAME_STYLE));

    /// Destructor, uninits the aui manager.
    virtual ~managed_frame();

    /// Virtual interface

    /// Returns true if the page can be closed.
    /// Default resets the find focus.
    virtual bool allow_close(
      /// notebook id
      wxWindowID id,
      /// page
      wxWindow* page);

    /// Executes a ex command. Returns true if
    /// this command is handled. This method is invoked
    /// at the beginning of the ex command handling,
    /// allowing you to override any command.
    virtual bool exec_ex_command(ex_command& command) { return false; };

    /// Called if the notebook changed page.
    /// Default sets the focus to page and adds page as recently used.
    virtual void on_notebook(wxWindowID id, wxWindow* page);

    /// Prints text in ex dialog.
    virtual void print_ex(
      /// the ex for the dialog
      ex* ex,
      /// the text to be printed
      const std::string& text);

    /// Allows you to perform action for a (vi) command.
    /// This method is invoked after command is executed.
    virtual void record(const std::string& command) { ; };

    /// Restores a previous saved current page.
    /// Returns restored page (default returns nullptr).
    virtual stc* restore_page(const std::string& key) { return nullptr; };

    /// Saves the current page, to restore later on.
    virtual bool save_current_page(const std::string& key) { return false; };

    /// Called after you checked the Sync checkbox on the options toolbar.
    /// Default syncs current stc.
    virtual void sync_all();

    /// Called after all pages from the notebooks are deleted.
    /// Default resets the find focus.
    virtual void sync_close_all(wxWindowID id);

    /// overridden methods

    stc* open_file(const path& filename, const data::stc& data = data::stc())
      override;

    void set_recent_file(const path& path) override;

    void statusbar_clicked(const std::string&) override;

    void statusbar_clicked_right(const std::string&) override;

    /// Other methods

    /// Returns file history.
    auto& file_history() { return m_file_history; };

    /// Debugging interface.
    auto* get_debug() { return m_debug; };

    /// Returns the find toolbar.
    auto* get_find_toolbar() { return m_findbar; };

    /// Returns the options toolbar.
    auto* get_options_toolbar() { return m_optionsbar; };

    /// Returns the toolbar.
    auto* get_toolbar() { return m_toolbar; };

    /// Hides the ex bar.
    /// Default it sets focus back to stc component associated with current ex.
    void hide_ex_bar(int hide = HIDE_BAR_FOCUS_STC);

    /// Add panes to the manager.
    /// Returns false if one of the panes could not be added.
    bool pane_add(
      /// panes
      const panes_t& panes,
      /// name of perspective to load / save
      const std::string& perspective = "managed frame");

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
    size_t panes() { return m_manager.GetAllPanes().GetCount(); };

    /// Returns a command line ex command.
    /// Shows the ex bar, sets the label and sets focus to it, allowing
    /// you to enter a command.
    /// Returns false if label is not supported.
    bool show_ex_command(
      /// the ex on which command is to be done
      ex* ex,
      /// label for the ex bar (/, ?, :, =)
      const std::string& label);

    /// Shows ex bar, and enters input mode.
    /// Returns false if command is not supported.
    bool show_ex_input(
      /// the ex on which command is to be done
      ex* ex,
      /// the command (a, c, or i)
      char command);

    /// Shows text in ex bar.
    void show_ex_message(const std::string& text);

    /// Shows or hide process pane.
    void show_process(bool show);

    /// Returns the toggled panes.
    const auto& toggled_panes() const { return m_toggled_panes; };

  protected:
    void on_menu_history(
      const class file_history& history,
      size_t                    index,
      data::stc::window_t       flags = 0);

  private:
    bool     add_toolbar_panes(const panes_t& panes);
    wxPanel* create_ex_panel();

    std::string m_perspective;

    const toggled_panes_t m_toggled_panes;

    debug* m_debug{nullptr};

    toolbar *m_findbar, *m_optionsbar, *m_toolbar;

    wxAuiManager       m_manager;
    class file_history m_file_history;
    textctrl*          m_textctrl;
  };
}; // namespace wex
