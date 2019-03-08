////////////////////////////////////////////////////////////////////////////////
// Name:      managedframe.h
// Purpose:   Declaration of wex::managed_frame class.
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <utility>
#include <vector>
#include <wx/aui/framemanager.h> // for wxAuiManager
#include <wx/aui/auibar.h>
#include <wex/defs.h>
#include <wex/filehistory.h>
#include <wex/frame.h>
#include <wex/path.h>

class wxPanel;

namespace wex
{
  class debug;
  class ex;
  class ex_command;
  class find_toolbar;
  class textctrl;
  class options_toolbar;
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
    
    /// Default constructor, registers the aui manager, and creates the bars.
    managed_frame(
      size_t maxFiles = 9,
      const window_data& data = window_data().style(wxDEFAULT_FRAME_STYLE));

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
    virtual bool exec_ex_command(ex_command& command) {return false;};

    /// Called if the notebook changed page.
    /// Default sets the focus to page and adds page as recently used.
    virtual void on_notebook(wxWindowID id, wxWindow* page);

    /// Interface from frame.
    virtual stc* open_file(
      const path& filename,
      const stc_data& data = stc_data()) override;

    /// Prints text in ex dialog.
    virtual void print_ex(
      /// the ex for the dialog
      ex* ex,
      /// the text to be printed
      const std::string& text);

    /// Restores a previous saved current page.
    /// Returns restored page (default returns nullptr).
    virtual stc* restore_page(const std::string& key) {return nullptr;};
    
    /// Saves the current page, to restore later on.
    virtual bool save_current_page(const std::string& key) {return false;};
    
    /// Allows derived class to update file history.
    virtual void set_recent_file(const path& path) override;
    
    /// Called after you checked the Sync checkbox on the options toolbar.
    /// Default syncs current stc.
    virtual void sync_all();

    /// Called after all pages from the notebooks are deleted.
    /// Default resets the find focus.
    virtual void sync_close_all(wxWindowID id);

    /// Other methods
    
    /// Appends the toggle panes to the specified menu.
    void append_panes(wxMenu* menu) const;
    
    /// Returns file history.
    auto& file_history() {return m_FileHistory;};
    
    /// Debugging interface.
    auto* get_debug() {return m_Debug;};

    /// Returns the find toolbar.
    auto* get_find_toolbar() {return m_FindBar;};
    
    /// Returns the options toolbar.
    auto* get_options_toolbar() {return m_OptionsBar;};
    
    /// Returns the toolbar.
    auto* get_toolbar() {return m_ToolBar;};
    
    /// Hides the ex bar.
    /// Default it sets focus back to stc component associated with current ex.
    void hide_ex_bar(int hide = HIDE_BAR_FOCUS_STC);

    /// Returns the manager.
    auto& manager() {return m_Manager;};

    /// Returns a command line ex command.
    /// Shows the ex bar, sets the label and sets focus to it, allowing
    /// you to enter a command.
    /// Returns false if label is not supported.
    bool show_ex_command(
      /// the ex on which command is to be done
      ex* ex, 
      /// label for the ex bar (/, ?, :, =)
      const std::string& label);
    
    /// Shows text in ex bar.
    void show_ex_message(const std::string& text);
    
    /// Shows or hides the managed pane.
    /// Returns false if pane is not managed.
    bool show_pane(const std::string& pane, bool show = true);
    
    /// Toggles the managed pane: if shown hides it, otherwise shows it.
    /// Returns false if pane is not managed.
    bool toggle_pane(
      const std::string& pane) {
        return show_pane(pane, !m_Manager.GetPane(pane).IsShown());};
  protected:
    void on_menu_history(
      const class file_history& history, 
      size_t index, 
      stc_data::window_t flags = 0);
  private:
    bool add_toolBarPane(
      wxWindow* window, 
      const std::string& name, 
      const wxString& caption = wxEmptyString);
    wxPanel* CreateExPanel();
    
    const std::vector<std::pair<std::pair<std::string, std::string>, int>> 
      m_ToggledPanes;
    
    wxAuiManager m_Manager;
    debug* m_Debug = nullptr;
    class file_history m_FileHistory;
    find_toolbar* m_FindBar;
    options_toolbar* m_OptionsBar;
    textctrl* m_TextCtrl;
    toolbar* m_ToolBar;
  };
};