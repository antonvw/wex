////////////////////////////////////////////////////////////////////////////////
// Name:      managedframe.h
// Purpose:   Declaration of wex::managed_frame class.
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
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
    /// Enums for HideExBar.
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
      const window_data& data = window_data().Style(wxDEFAULT_FRAME_STYLE));

    /// Destructor, uninits the aui manager.
    virtual ~managed_frame();

    /// Returns true if the page can be closed.
    /// Default resets the find focus.
    virtual bool AllowClose(
      /// notebook id
      wxWindowID id, 
      /// page
      wxWindow* page);
    
    /// Appends the toggle panes to the specified menu.
    void AppendPanes(wxMenu* menu) const;
    
    /// Executes a ex command. Returns true if
    /// this command is handled. This method is invoked
    /// at the beginning of the ex command handling,
    /// allowing you to override any command.
    virtual bool ExecExCommand(ex_command& command) {return false;};

    /// Debugging interface.
    auto* GetDebug() {return m_Debug;};

    /// Returns a command line ex command.
    /// Shows the ex bar, sets the label and sets focus to it, allowing
    /// you to enter a command.
    /// Returns false if label is not supported.
    bool GetExCommand(
      /// the ex on which command is to be done
      ex* ex, 
      /// label for the ex bar (/, ?, :, =)
      const std::string& label);
    
    /// Returns file history.
    auto & GetFileHistory() {return m_FileHistory;};
    
    /// Returns the find toolbar.
    auto * GetFindToolBar() {return m_FindBar;};
    
    /// Returns the manager.
    auto & GetManager() {return m_Manager;};

    /// Returns the options toolbar.
    auto * GetOptionsToolBar() {return m_OptionsBar;};
    
    /// Returns the toolbar.
    auto * GetToolBar() {return m_ToolBar;};
    
    /// Hides the ex bar.
    /// Default it sets focus back to stc component associated with current ex.
    void HideExBar(int hide = HIDE_BAR_FOCUS_STC);

    /// Called if the notebook changed page.
    /// Default sets the focus to page and adds page as recently used.
    virtual void OnNotebook(wxWindowID id, wxWindow* page);

    /// Interface from frame.
    virtual stc* OpenFile(
      const path& filename,
      const stc_data& data = stc_data()) override;

    /// Prints text in ex dialog.
    virtual void PrintEx(
      /// the ex for the dialog
      ex* ex,
      /// the text to be printed
      const std::string& text);

    /// Restores a previous saved current page.
    /// Returns restored page (default returns nullptr).
    virtual stc* RestorePage(const std::string& key) {return nullptr;};
    
    /// Saves the current page, to restore later on.
    virtual bool SaveCurrentPage(const std::string& key) {return false;};
    
    /// Allows derived class to update file history.
    virtual void SetRecentFile(const path& path) override;
    
    /// Shows text in ex bar.
    void ShowExMessage(const std::string& text);
    
    /// Shows or hides the managed pane.
    /// Returns false if pane is not managed.
    bool ShowPane(const std::string& pane, bool show = true);
    
    /// Called after you checked the Sync checkbox on the options toolbar.
    /// Default syncs current stc.
    virtual void SyncAll();

    /// Called after all pages from the notebooks are deleted.
    /// Default resets the find focus.
    virtual void SyncCloseAll(wxWindowID id);

    /// Toggles the managed pane: if shown hides it, otherwise shows it.
    /// Returns false if pane is not managed.
    bool TogglePane(
      const std::string& pane) {
        return ShowPane(pane, !m_Manager.GetPane(pane).IsShown());};
  protected:
    void DoRecent(
      const file_history& history, 
      size_t index, 
      stc_data::window_flags flags = stc_data::WIN_DEFAULT);
  private:
    bool AddToolBarPane(
      wxWindow* window, 
      const std::string& name, 
      const wxString& caption = wxEmptyString);
    wxPanel* CreateExPanel();
    
    const std::vector<std::pair<std::pair<std::string, std::string>, int>> 
      m_ToggledPanes;
    
    wxAuiManager m_Manager;
    debug* m_Debug = nullptr;
    file_history m_FileHistory;
    find_toolbar* m_FindBar;
    options_toolbar* m_OptionsBar;
    textctrl* m_TextCtrl;
    toolbar* m_ToolBar;
  };
};
