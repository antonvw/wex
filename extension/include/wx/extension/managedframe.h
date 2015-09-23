////////////////////////////////////////////////////////////////////////////////
// Name:      managedframe.h
// Purpose:   Declaration of wxExManagedFrame class.
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/aui/framemanager.h> // for wxAuiManager
#include <wx/aui/auibar.h>
#include <wx/extension/filehistory.h>
#include <wx/extension/defs.h>
#include <wx/extension/frame.h>

// Only if we have a gui.
#if wxUSE_GUI

class wxPanel;
class wxExEx;
class wxExExTextCtrl;
class wxExToolBar;

/// Offers an aui managed frame with a notebook multiple document interface,
/// used by the notebook classes, and toolbar, findbar and vibar support.
/// - The toolbar and findbar are added as wxExToolbarPanes to the aui manager.
/// - The vibar is added as normal aui panel to the aui manager.
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
  
  /// Constructor, registers the aui manager, and creates the bars.
  wxExManagedFrame(wxWindow* parent,
    wxWindowID id,
    const wxString& title,
    size_t maxFiles = 9,
    long style = wxDEFAULT_FRAME_STYLE);

  /// Destructor, uninits the aui manager.
 ~wxExManagedFrame();

  /// Returns true if the page can be closed.
  /// Default resets the find focus.
  virtual bool AllowClose(
    /// notebook id
    wxWindowID id, 
    /// page
    wxWindow* page);
  
  /// Executes a ex command.
  virtual bool ExecExCommand(int command) {return NULL;};

  /// Gets a command line ex command.
  /// Default shows the ex bar, sets the label and 
  /// sets focus to it, allowing
  /// you to enter a command.
  /// You can override it to e.g. hide other panels.
  virtual void GetExCommand(
    /// the ex on which command is to be done
    wxExEx* ex, 
    /// label for the ex bar (like / or ? or :)
    const wxString& label);
  
  /// Returns file history.
  wxExFileHistory& GetFileHistory() {return m_FileHistory;};
  
  /// Gets the manager.
  wxAuiManager& GetManager() {return m_Manager;};

  /// Returns the toolbar.
  wxExToolBar* GetToolBar() {return m_ToolBar;};
  
  /// Hides the ex bar.
  /// Default it sets focus back to stc component associated with current ex.
  void HideExBar(int hide = HIDE_BAR_FOCUS_STC);

  /// Called if the notebook changed page.
  /// Default sets the focus to page and adds page as recently used.
  virtual void OnNotebook(wxWindowID id, wxWindow* page);

  /// Interface from wxExFrame.
  virtual bool OpenFile(
    const wxExFileName& filename,
    int line_number = 0,
    const wxString& match = wxEmptyString,
    int col_number = 0,
    long flags = 0) override;

  /// Prints text in ex dialog.
  virtual void PrintEx(
    /// the ex for the dialog
    wxExEx* ex,
    /// the text to be printed
    const wxString& text);
  
  virtual bool SetRecentFile(const wxString& file) override {
    return m_FileHistory.SetRecentFile(file);};
  
  /// Shows text in ex bar.
  void ShowExMessage(const wxString& text);
  
  /// Called after you checked the Sync checkbox on the options toolbar.
  /// Default syncs current stc.
  virtual void SyncAll();

  /// Called after all pages from the notebooks are deleted.
  /// Default resets the find focus.
  virtual void SyncCloseAll(wxWindowID id);

  /// Toggles the managed pane: if shown hides it, otherwise shows it.
  /// Returns false if pane is not managed.
  bool TogglePane(
    /// pane to be toggled:
    /// - FINDBAR
    /// - OPTIONSBAR
    /// - TOOLBAR
    /// - VIBAR (same as the ex bar)
    const wxString& pane);
protected:
  void DoRecent(wxFileHistory& history, size_t index, long flags = 0);
private:
  bool AddToolBarPane(
    wxWindow* window, 
    const wxString& name, 
    const wxString& caption = wxEmptyString);
  wxPanel* CreateExPanel();
  
  wxAuiManager m_Manager;
  wxExFileHistory m_FileHistory;
  wxExToolBar* m_ToolBar;
  wxExExTextCtrl* m_exTextCtrl;
};
#endif // wxUSE_GUI
