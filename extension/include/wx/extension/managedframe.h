////////////////////////////////////////////////////////////////////////////////
// Name:      managedframe.h
// Purpose:   Declaration of wxExManagedFrame class.
// Author:    Anton van Wezenbeek
// Created:   2010-04-11
// Copyright: (c) 2010 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXMANAGEDFRAME_H
#define _EXMANAGEDFRAME_H

#include <wx/aui/framemanager.h> // for wxAuiManager
#include <wx/extension/frame.h>

// Only if we have a gui.
#if wxUSE_GUI

class wxStaticText;
class wxExToolBar;
class wxExVi;
class wxExViTextCtrl;

/// Offers an aui managed frame with a notebook multiple document interface,
/// used by the notebook classes, and toolbar, findbar and vibar support.
/// - The toolbar, and findbar are added as wxExToolbarPanes to the aui manager.
///   You can add your own controls to the toolbar by overriding the DoAddControl.
/// - The vi bars are added as panels to the aui manager.
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

  // Interface for notebooks.
  /// Returns true if the page can be closed.
  virtual bool AllowClose(
    wxWindowID WXUNUSED(id), 
    wxWindow* WXUNUSED(page)) {return true;}

  /// Called if the notebook changed page.
  virtual void OnNotebook(
    wxWindowID WXUNUSED(id), 
    wxWindow* WXUNUSED(page)) {;};

  /// Called after all pages from the notebooks are deleted.
  virtual void SyncCloseAll(wxWindowID WXUNUSED(id)) {;};

  /// Gets the manager.
  wxAuiManager& GetManager() {return m_Manager;};

  /// Gets a command line vi command.
  void GetViCommand(wxExVi* vi, const wxString& command);
  
  /// Returns true if vi command is a find command.
  bool GetViCommandIsFind() const;
  
  /// Returns true if vi command is a find command and a find next.
  bool GetViCommandIsFindNext() const;
  
  /// Hides the vi bar.
  void HideViBar();

  /// Shows text in vi bar.
  void ShowViMessage(const wxString& text);
  
  /// Toggles the managed pane: if shown hides it, otherwise shows it.
  void TogglePane(const wxString& pane);
protected:
  /// Add controls to specified toolbar.
  /// This toolbar already contains the standard controls.
  virtual void DoAddControl(wxExToolBar*) {;};

  void OnCommand(wxCommandEvent& event);
  void OnUpdateUI(wxUpdateUIEvent& event);
private:
  bool AddToolBarPane(
    wxWindow* window, 
    const wxString& name, 
    const wxString& caption = wxEmptyString);
  
  wxAuiManager m_Manager;

  wxExViTextCtrl* m_viTextCtrl;
  wxStaticText* m_viTextPrefix;
  
  DECLARE_EVENT_TABLE()
};
#endif // wxUSE_GUI
#endif
