////////////////////////////////////////////////////////////////////////////////
// Name:      frame.h
// Purpose:   Declaration of wxExFrame class
// Author:    Anton van Wezenbeek
// RCS-ID:    $Id$
// Created:   2010-03-26
// Copyright: (c) 2010 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXFRAME_H
#define _EXFRAME_H

#include <vector>
#include <wx/fdrepdlg.h> // for wxFindDialogDialog and Event
#include <wx/frame.h>
#include <wx/extension/statusbar.h>
#include <wx/extension/defs.h> // for ID_EDIT_STATUS_BAR
#include <wx/extension/filename.h>
#include <wx/extension/vcsentry.h>

// Only if we have a gui.
#if wxUSE_GUI

class wxExGrid;
class wxExListView;
class wxExSTC;
class wxExToolBar;

/// Offers a frame with easy statusbar methods, 
/// find/replace, and allows for file dropping.
/// Also helps in maintaining focus to the base controls
/// (grid, listview and STC).
class WXDLLIMPEXP_BASE wxExFrame : public wxFrame
{
public:
  /// Constructor, the frame position and size is taken from the config,
  /// the name is used internally for persistent registration.
  wxExFrame(wxWindow* parent,
    wxWindowID id,
    const wxString& title,
    long style = wxDEFAULT_FRAME_STYLE);

  /// Destructor, deletes the statusbar.
 ~wxExFrame();

  /// Invokes GetFindString on one of the controls.
  const wxString GetFindString();

  /// Returns a grid, default returns the focused grid.
  virtual wxExGrid* GetGrid() {return m_FocusGrid;};

  /// Returns a listview, default returns the focused listview.
  virtual wxExListView* GetListView() {return m_FocusListView;};

  /// Returns an STC, default returns the focused STC.
  virtual wxExSTC* GetSTC() {return m_FocusSTC;};

  /// Gets the focused grid.
  wxExGrid* GetFocusedGrid() {return m_FocusGrid;};

  /// Gets the focused list view.
  wxExListView* GetFocusedListView() {return m_FocusListView;};

  /// Gets the focused STC.
  wxExSTC* GetFocusedSTC() {return m_FocusSTC;};

  /// Called when a config dialog command event is triggered.
  /// Default it fires when the apply button was pressed.
  virtual void OnCommandConfigDialog(
    wxWindowID WXUNUSED(dialogid),
    int WXUNUSED(commandid) = wxID_APPLY) {};

  /// Default opens the file using the GetFocusedSTC.
  virtual bool OpenFile(
    const wxExFileName& filename,
    int line_number = 0,
    const wxString& match = wxEmptyString,
    long flags = 0);

  /// Allows you to open a filename with info from vcs.
  virtual bool OpenFile(
    const wxExFileName& filename,
    const wxExVCSEntry& vcs,
    long flags = 0);
    
  /// Override from base class.
  void SetMenuBar(wxMenuBar* bar);

#if wxUSE_STATUSBAR
  /// Sets up the status bar if you want to use StatusText.
  /// The first pane should be reserved for display status text messages.
  /// The next panes are used by the framework:
  /// - PaneFileType, shows file types
  /// - PaneInfo, shows info for control, e.g. lines
  /// - PaneLexer, shows lexer
  void SetupStatusBar(
    const std::vector<wxExStatusBarPane>& panes,
    long style = wxST_SIZEGRIP,
    wxWindowID id = ID_EDIT_STATUS_BAR,
    const wxString& name = "statusBar");

  /// When (left) double clicked, uses the GetSTC() for some dialogs.
  virtual void StatusBarDoubleClicked(
    const wxString& pane);

  /// Do something when statusbar is (right) double clicked.
  virtual void StatusBarDoubleClickedRight(const wxString& ) {};

  /// Sets text on specified pane.
  /// Don't forget to call SetupStatusBar first.
  static void StatusText(const wxString& text, const wxString& pane);
#endif // wxUSE_STATUSBAR

protected:
  /// Handles command event.
  void OnCommand(wxCommandEvent& command);

#if wxUSE_STATUSBAR
  // Interface from wxFrame.
  virtual wxStatusBar* OnCreateStatusBar(int number,
    long style,
    wxWindowID id,
    const wxString& name);
#endif

  /// If there is a STC, calls find.
  void OnFindDialog(wxFindDialogEvent& event);
  
  /// If there is a focused STC, updates the status bar.
  void OnUpdateUI(wxUpdateUIEvent& event);  
private:
  void FindIn(wxFindDialogEvent& event, wxExGrid* grid);
  void FindIn(wxFindDialogEvent& event, wxExListView* lv);
  void FindIn(wxFindDialogEvent& event, wxExSTC* stc);
  void Initialize();

#if wxUSE_STATUSBAR
  static wxExStatusBar* m_StatusBar;
#endif

  wxFindReplaceDialog* m_FindReplaceDialog;
  wxMenuBar* m_MenuBar;

  wxExGrid* m_FocusGrid;
  wxExListView* m_FocusListView;
  wxExSTC* m_FocusSTC;
  wxExSTC* m_FocusSTCFind; // focs before find dlg was activated
  
  bool m_IsCommand;

  DECLARE_EVENT_TABLE()
};
#endif // wxUSE_GUI
#endif
