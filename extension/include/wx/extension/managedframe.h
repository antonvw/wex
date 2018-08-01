////////////////////////////////////////////////////////////////////////////////
// Name:      managedframe.h
// Purpose:   Declaration of wxExManagedFrame class.
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <utility>
#include <vector>
#include <wx/aui/framemanager.h> // for wxAuiManager
#include <wx/aui/auibar.h>
#include <wx/extension/filehistory.h>
#include <wx/extension/defs.h>
#include <wx/extension/frame.h>
#include <wx/extension/path.h>

class wxPanel;
class wxExExCommand;
class wxExDebug;
class wxExEx;
class wxExFindToolBar;
class wxExTextCtrl;
class wxExOptionsToolBar;
class wxExToolBar;

/// Offers an aui managed frame with a notebook multiple document interface,
/// used by the notebook classes, and toolbar, findbar and vibar support.
/// - The toolbar and findbar are added as wxExToolbarPanes to the aui manager.
/// - The vibar is added as normal aui panel to the aui manager.
/// The next panes are supported:
/// - FINDBAR
/// - OPTIONSBAR
/// - PROCESS
/// - TOOLBAR
/// - VIBAR (same as the ex bar)
class WXDLLIMPEXP_BASE wxExManagedFrame : public wxExFrame
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
  wxExManagedFrame(
    size_t maxFiles = 9,
    const wxExWindowData& data = wxExWindowData().Style(wxDEFAULT_FRAME_STYLE));

  /// Destructor, uninits the aui manager.
  virtual ~wxExManagedFrame();

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
  /// at the beginning of the wxExEx command handling,
  /// allowing you to override any command.
  virtual bool ExecExCommand(wxExExCommand& command) {return false;};

  /// Debugging interface.
  auto* GetDebug() {return m_Debug;};

  /// Returns a command line ex command.
  /// Shows the ex bar, sets the label and sets focus to it, allowing
  /// you to enter a command.
  /// Returns false if label is not supported.
  bool GetExCommand(
    /// the ex on which command is to be done
    wxExEx* ex, 
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

  /// Interface from wxExFrame.
  virtual wxExSTC* OpenFile(
    const wxExPath& filename,
    const wxExSTCData& stc_data = wxExSTCData()) override;

  /// Prints text in ex dialog.
  virtual void PrintEx(
    /// the ex for the dialog
    wxExEx* ex,
    /// the text to be printed
    const std::string& text);

  /// Restores a previous saved current page.
  /// Returns restored page (default returns nullptr).
  virtual wxExSTC* RestorePage(const std::string& key) {return nullptr;};
  
  /// Saves the current page, to restore later on.
  virtual bool SaveCurrentPage(const std::string& key) {return false;};
  
  /// Allows derived class to update file history.
  virtual void SetRecentFile(const wxExPath& path) override;
  
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
    const wxExFileHistory& history, 
    size_t index, 
    wxExSTCWindowFlags flags = STC_WIN_DEFAULT);
private:
  bool AddToolBarPane(
    wxWindow* window, 
    const std::string& name, 
    const wxString& caption = wxEmptyString);
  wxPanel* CreateExPanel();
  
  const std::vector<std::pair<std::pair<std::string, std::string>, int>> 
    m_ToggledPanes;
  
  wxAuiManager m_Manager;
  wxExDebug* m_Debug = nullptr;
  wxExFileHistory m_FileHistory;
  wxExFindToolBar* m_FindBar;
  wxExOptionsToolBar* m_OptionsBar;
  wxExTextCtrl* m_TextCtrl;
  wxExToolBar* m_ToolBar;
};
