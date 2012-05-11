////////////////////////////////////////////////////////////////////////////////
// Name:      managedframe.h
// Purpose:   Declaration of wxExManagedFrame class.
// Author:    Anton van Wezenbeek
// Copyright: (c) 2012 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXMANAGEDFRAME_H
#define _EXMANAGEDFRAME_H

#include <wx/aui/auibar.h> // for wxAuiToolBar
#include <wx/aui/framemanager.h> // for wxAuiManager
#include <wx/panel.h>
#include <wx/extension/frame.h>

// Only if we have a gui.
#if wxUSE_GUI

class wxStaticText;
class wxExToolBar;
class wxExEx;
class wxExExTextCtrl;

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
  virtual bool AllowClose(wxWindowID id, wxWindow* page);
  
  /// Executes a ex command that can result
  /// in changing stc, if command is being played back.
  virtual wxExSTC* ExecExCommand(int command) {return NULL;};

  /// Gets a command line vi command.
  /// It shows the vibar, sets the label and 
  /// sets focus to it, allowing
  /// you to enter a command.
  /// You can override it to e.g. hide other panels.
  virtual void GetExCommand(
    /// the ex on which command is to be done
    wxExEx* ex, 
    /// label for the vibar (like / or ? or :)
    const wxString& label);
  
  /// Gets the manager.
  wxAuiManager& GetManager() {return m_Manager;};

  /// Hides the vi bar.
  /// Default it sets focus back to stc component associated with current vi.
  void HideExBar(bool set_focus = true);

  /// Called if the notebook changed page.
  virtual void OnNotebook(wxWindowID id, wxWindow* page);

  /// Shows text in vi bar.
  void ShowExMessage(const wxString& text);
  
  /// Called after all pages from the notebooks are deleted.
  virtual void SyncCloseAll(wxWindowID id);

  /// Toggles the managed pane: if shown hides it, otherwise shows it.
  /// Returns false if pane is not managed.
  bool TogglePane(
    /// FINDBAR, TOOLBAR, VIBAR  
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
  wxStaticText* m_exTextPrefix;
  
  DECLARE_EVENT_TABLE()
};
#endif // wxUSE_GUI
#endif
