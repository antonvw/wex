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
class wxExFrame : public wxFrame
{
public:
  /// Flags for StatusText.
  enum wxExStatusFlags
  {
    STAT_DEFAULT  = 0x0000, ///< shows 'modified' and file 'fullname'
    STAT_SYNC     = 0x0001, ///< shows 'synchronized' instead of 'modified'
    STAT_FULLPATH = 0x0002, ///< shows file 'fullpath' instead of 'fullname'
  };

  /// Constructor, the frame position and size is taken from the config.
  wxExFrame(wxWindow* parent,
    wxWindowID id,
    const wxString& title,
    long style = wxDEFAULT_FRAME_STYLE,
    const wxString& name = wxFrameNameStr);

  /// Destructor, deletes the statusbar.
 ~wxExFrame();

  /// Invokes GetFindString on one of the controls.
  void GetFindString();

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

  /// Allows you to open a filename with specified contents.
  /// The unique argument can be used as addition for a key in the notebook.
  virtual bool OpenFile(
    const wxExFileName& WXUNUSED(filename),
    const wxString& WXUNUSED(unique),
    const wxString& WXUNUSED(contents),
    long flags = 0) {return false;};

#if wxUSE_STATUSBAR
  /// Do something when statusbar is clicked.
  virtual void StatusBarClicked(const wxString& ) {};

  /// When double clicked, uses the GetSTC() for some dialogs.
  virtual void StatusBarDoubleClicked(
    const wxString& pane);

  /// Sets text on specified pane.
  /// Don't forget to call SetupStatusBar first.
  static void StatusText(
    const wxString& text, 
    const wxString& pane = "PaneText");

  /// Shows filename info on the statusbar.
  // Using type wxExStatusFlags instead of long gives compiler errors at
  // invoking.
  static void StatusText(
    const wxExFileName& filename, 
    long flags = STAT_DEFAULT);
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
  
#if wxUSE_STATUSBAR
  /// Sets up the status bar if you want to use StatusText.
  void SetupStatusBar(
    const std::vector<wxExPane>& panes,
    long style = wxST_SIZEGRIP,
    wxWindowID id = ID_EDIT_STATUS_BAR,
    const wxString& name = "statusBar");
#endif

private:
  void FindIn(wxFindDialogEvent& event, wxExGrid* grid);
  void FindIn(wxFindDialogEvent& event, wxExListView* lv);
  void FindIn(wxFindDialogEvent& event, wxExSTC* stc);
  void Initialize();

#if wxUSE_STATUSBAR
  static wxExStatusBar* m_StatusBar;
#endif

  wxFindReplaceDialog* m_FindReplaceDialog;

  wxExGrid* m_FocusGrid;
  wxExListView* m_FocusListView;
  wxExSTC* m_FocusSTC;

  DECLARE_EVENT_TABLE()
};
#endif // wxUSE_GUI
#endif
