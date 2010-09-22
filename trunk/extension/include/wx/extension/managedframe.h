////////////////////////////////////////////////////////////////////////////////
// Name:      managedframe.h
// Purpose:   Declaration of wxExManagedFrame class.
// Author:    Anton van Wezenbeek
// Created:   2010-04-11
// RCS-ID:    $Id$
// Copyright: (c) 2010 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXMANAGEDFRAME_H
#define _EXMANAGEDFRAME_H

#include <wx/aui/framemanager.h> // for wxAuiManager
#include <wx/extension/frame.h>

// Only if we have a gui.
#if wxUSE_GUI

class wxExToolBar;

#if wxUSE_AUI
/// Offers an aui managed frame with a notebook multiple document interface,
/// used by the notebook classes, and toolbar and findbar support.
class wxExManagedFrame : public wxExFrame
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

  /// Toggles the managed pane: if shown hides it, otherwise shows it.
  void TogglePane(const wxString& pane);
protected:
  /// Add controls to specified toolbar.
  virtual void DoAddControl(wxExToolBar*) {;};

  void OnCommand(wxCommandEvent& event);
  void OnUpdateUI(wxUpdateUIEvent& event);
private:
  wxAuiManager m_Manager;

  DECLARE_EVENT_TABLE()
};
#endif // wxUSE_AUI
#endif // wxUSE_GUI
#endif
