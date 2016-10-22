////////////////////////////////////////////////////////////////////////////////
// Name:      frame.h
// Purpose:   Declaration of wxExFrame class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <vector>
#include <wx/frame.h>
#include <wx/extension/statusbar.h>

// Only if we have a gui.
#if wxUSE_GUI

class wxFindReplaceDialog;
class wxListView;

class wxExFileName;
class wxExGrid;
class wxExListView;
class wxExProcess;
class wxExSTC;
class wxExVCSEntry;

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

  /// Destructor.
  virtual ~wxExFrame();

  /// Returns a grid.
  virtual wxExGrid* GetGrid();

  /// Returns a listview.
  virtual wxExListView* GetListView();

  /// Returns an STC.
  virtual wxExSTC* GetSTC();

  /// Returns true if file is opened in a window.
  virtual bool IsOpen(const wxExFileName& filename);

  /// Called when an item dialog command event is triggered.
  virtual void OnCommandItemDialog(
    wxWindowID WXUNUSED(dialogid),
    const wxCommandEvent& WXUNUSED(event)) {};

  /// Default opens the file using GetSTC.
  /// Returns stc component opened, or nullptr.
  virtual wxExSTC* OpenFile(
    const wxExFileName& filename,
    int line_number = 0,
    const std::string& match = std::string(),
    int col_number = 0,
    long flags = 0,
    const std::string& command = std::string());

  /// Allows you to open a filename with info from vcs.
  /// Returns stc component opened, or nullptr.
  virtual wxExSTC* OpenFile(
    const wxExFileName& filename,
    const wxExVCSEntry& vcs,
    long flags = 0);
    
  /// Allows you to open a filename with specified contents.
  /// Returns stc component opened, or nullptr.
  virtual wxExSTC* OpenFile(
    const wxExFileName& filename,
    const std::string& text,
    long flags = 0);
  
  /// Allows you to e.g. add debugging.
  /// Default returns nullptr.
  virtual wxExProcess* Process(const std::string& command) {return nullptr;};
    
  /// Sets the find focus to specified window.
  void SetFindFocus(wxWindow* focus) {m_FindFocus = focus;};
  
  /// Override from base class.
  virtual void SetMenuBar(wxMenuBar* bar) override;
  
  /// Allows derived class to update file history.
  virtual void SetRecentFile(const wxString& file) {;};

#if wxUSE_STATUSBAR
  /// Sets up the status bar if you want to use StatusText.
  /// The first pane should be reserved for display status text messages.
  /// The next panes are used by the framework:
  /// - PaneFileType, shows file types
  /// - PaneInfo, shows info for control, e.g. lines
  /// - PaneLexer, shows lexer
  /// Returns created statusbar.
  wxExStatusBar* SetupStatusBar(
    const std::vector<wxExStatusBarPane>& panes,
    long style = wxST_SIZEGRIP,
    const wxString& name = "statusBar");

  /// When (left) clicked, uses the GetSTC() for some dialogs.
  virtual void StatusBarClicked(
    const wxString& pane);

  /// Do something when statusbar is (right) clicked.
  virtual void StatusBarClickedRight(const wxString& ) {};

  /// Returns text on specified pane.
  /// Don't forget to call SetupStatusBar first.
  static wxString GetStatusText(const wxString& pane);
  
  /// Sets text on specified pane.
  /// Don't forget to call SetupStatusBar first.
  static bool StatusText(const wxString& text, const wxString& pane);
  
  /// Updates statusbar pane items pane with values from specified listview.
  static bool UpdateStatusBar(const wxListView* lv);
  
  /// Updates the specified statusbar pane with values from specified stc.
  static bool UpdateStatusBar(wxExSTC* stc, const wxString& pane);
#endif // wxUSE_STATUSBAR
protected:
#if wxUSE_STATUSBAR
  // Interface from wxFrame.
  virtual wxStatusBar* OnCreateStatusBar(int number,
    long style, wxWindowID id, const wxString& name) override;
#endif

#if wxUSE_STATUSBAR
  static wxExStatusBar* m_StatusBar;
#endif
private:
  wxWindow* m_FindFocus = nullptr;
  wxFindReplaceDialog* m_FindReplaceDialog = nullptr;
  wxMenuBar* m_MenuBar = nullptr;
  
  bool m_IsCommand = false;
};
#endif // wxUSE_GUI
