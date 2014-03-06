////////////////////////////////////////////////////////////////////////////////
// Name:      managedframe.h
// Purpose:   Declaration of wxExManagedFrame class.
// Author:    Anton van Wezenbeek
// Copyright: (c) 2014 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXMANAGEDFRAME_H
#define _EXMANAGEDFRAME_H

#include <wx/aui/framemanager.h> // for wxAuiManager
#include <wx/extension/frame.h>

// Only if we have a gui.
#if wxUSE_GUI

class wxPane;
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
  /// Constructor, registers the aui manager, and creates the bars.
  wxExManagedFrame(wxWindow* parent,
    wxWindowID id,
    const wxString& title,
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
  
  /// Executes a ex command that can result
  /// in changing stc, if command is being played back.
  virtual wxExSTC* ExecExCommand(int command) {return NULL;};

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
  
  /// Gets the manager.
  wxAuiManager& GetManager() {return m_Manager;};

  /// Hides the ex bar.
  /// Default it sets focus back to stc component associated with current ex.
  void HideExBar(bool set_focus = true);

  /// Called if the notebook changed page.
  /// Default sets the focus to page.
  virtual void OnNotebook(wxWindowID id, wxWindow* page);

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
  /// Returns the toolbar.
  wxExToolBar* GetToolBar() {return m_ToolBar;};
  void OnCommand(wxCommandEvent& event);
  void OnUpdateUI(wxUpdateUIEvent& event);
private:
  bool AddToolBarPane(
    wxWindow* window, 
    const wxString& name, 
    const wxString& caption = wxEmptyString);
  wxPanel* CreateExPanel();
  
  wxAuiManager m_Manager;
  wxExToolBar* m_ToolBar;
  wxExExTextCtrl* m_exTextCtrl;
  
  DECLARE_EVENT_TABLE()
};
#endif // wxUSE_GUI
#endif
