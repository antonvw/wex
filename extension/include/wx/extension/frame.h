////////////////////////////////////////////////////////////////////////////////
// Name:      frame.h
// Purpose:   Declaration of wxExFrame class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXFRAME_H
#define _EXFRAME_H

#include <vector>
#include <wx/fdrepdlg.h> // for wxFindDialogDialog and Event
#include <wx/frame.h>
#include <wx/extension/statusbar.h>
#include <wx/extension/filename.h>
#include <wx/extension/vcsentry.h>

// Only if we have a gui.
#if wxUSE_GUI

class wxExGrid;
class wxExListView;
class wxExSTC;
class wxListView;

/// Offers a frame with easy statusbar methods, 
/// find/replace, and allows for file dropping.
/// Also helps in maintaining access to the base controls
/// (grid, listview and STC).
class WXDLLIMPEXP_BASE wxExFrame : public wxFrame
{
public:
  /// Constructor,
  /// the name is used internally for persistent registration,
  /// setting the frame position and size. 
  wxExFrame(wxWindow* parent,
    wxWindowID id,
    const wxString& title,
    long style = wxDEFAULT_FRAME_STYLE);

  /// Destructor, deletes the statusbar.
 ~wxExFrame();

  /// Returns a grid.
  virtual wxExGrid* GetGrid();

  /// Returns a listview.
  virtual wxExListView* GetListView();

  /// Returns an STC.
  virtual wxExSTC* GetSTC();

  /// Called when a config dialog command event is triggered.
  /// Default it fires when the apply button was pressed.
  virtual void OnCommandConfigDialog(
    wxWindowID WXUNUSED(dialogid),
    int WXUNUSED(commandid) = wxID_APPLY) {};

  /// Default opens the file using GetSTC.
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
    
  /// Sets the find focus to specified window.
  void SetFindFocus(wxWindow* focus);
  
  /// Override from base class.
  virtual void SetMenuBar(wxMenuBar* bar);

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
    const wxString& name = "statusBar");

  /// When (left) double clicked, uses the GetSTC() for some dialogs.
  virtual void StatusBarDoubleClicked(
    const wxString& pane);

  /// Do something when statusbar is (right) double clicked.
  virtual void StatusBarDoubleClickedRight(const wxString& ) {};

  /// Sets text on specified pane.
  /// Don't forget to call SetupStatusBar first.
  static void StatusText(const wxString& text, const wxString& pane);
  
  /// Call this if you think the find focus should be updated.
  void UpdateFindFocus();
  
  /// Updates statusbar pane items pane with values from specified listview.
  static void UpdateStatusBar(const wxListView* lv);
  
  /// Updates the specified statusbar pane with values from specified stc.
  static void UpdateStatusBar(wxExSTC* stc, const wxString& pane);
#endif // wxUSE_STATUSBAR
protected:
#if wxUSE_STATUSBAR
  // Interface from wxFrame.
  virtual wxStatusBar* OnCreateStatusBar(int number,
    long style,
    wxWindowID id,
    const wxString& name);
#endif

  void OnCommand(wxCommandEvent& command);
  void OnFindDialog(wxFindDialogEvent& event);
  void OnUpdateUI(wxUpdateUIEvent& event);  
private:
  void Initialize();

#if wxUSE_STATUSBAR
  static wxExStatusBar* m_StatusBar;
#endif

  wxWindow* m_FindFocus;
  wxFindReplaceDialog* m_FindReplaceDialog;
  wxMenuBar* m_MenuBar;
  
  bool m_IsCommand;

  DECLARE_EVENT_TABLE()
};
#endif // wxUSE_GUI
#endif
