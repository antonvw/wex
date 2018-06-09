////////////////////////////////////////////////////////////////////////////////
// Name:      frame.h
// Purpose:   Declaration of wxExFrame class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <vector>
#include <wx/frame.h>
#include <wx/extension/statusbar.h>
#include <wx/extension/stc-data.h>
#include <wx/extension/stc-enums.h>
#include <wx/extension/window-data.h>

// Only if we have a gui.
#if wxUSE_GUI

class wxFindReplaceDialog;
class wxListView;

class wxExPath;
class wxExGrid;
class wxExListView;
class wxExPath;
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
  /// Default constructor,
  wxExFrame(const wxExWindowData& data = wxExWindowData());

  /// Destructor.
  virtual ~wxExFrame();

  /// Returns a grid.
  virtual wxExGrid* GetGrid();

  /// Returns a listview.
  virtual wxExListView* GetListView();

  /// Returns an STC.
  virtual wxExSTC* GetSTC();

  /// Returns true if file is opened in a window.
  virtual bool IsOpen(const wxExPath& filename);

  /// Called when an item dialog command event is triggered.
  virtual void OnCommandItemDialog(
    wxWindowID WXUNUSED(dialogid),
    const wxCommandEvent& WXUNUSED(event)) {};

  /// Default opens the file using GetSTC.
  /// Returns stc component opened, or nullptr.
  virtual wxExSTC* OpenFile(
    const wxExPath& filename,
    const wxExSTCData& data = wxExSTCData());

  /// Allows you to open a filename with info from vcs.
  /// Returns stc component opened, or nullptr.
  virtual wxExSTC* OpenFile(
    const wxExPath& filename,
    const wxExVCSEntry& vcs,
    const wxExSTCData& data = wxExSTCData());
    
  /// Allows you to open a filename with specified contents.
  /// Returns stc component opened, or nullptr.
  virtual wxExSTC* OpenFile(
    const wxExPath& filename,
    const std::string& text,
    const wxExSTCData& data = wxExSTCData());
  
  /// Allows you to e.g. add debugging.
  /// Default returns nullptr.
  virtual wxExProcess* Process(const std::string& command) {return nullptr;};
    
  /// Sets the find focus to specified window.
  void SetFindFocus(wxWindow* focus) {m_FindFocus = focus;};
  
  /// Override from base class.
  virtual void SetMenuBar(wxMenuBar* bar) override;
  
  /// Allows derived class to update file history.
  virtual void SetRecentFile(const wxExPath& path) {;};

#if wxUSE_STATUSBAR
  /// Sets up the status bar if you want to use StatusText.
  wxExStatusBar* SetupStatusBar(
    const std::vector<wxExStatusBarPane>& panes,
    long style = wxST_SIZEGRIP,
    const wxString& name = "statusBar") {
    return wxExStatusBar::Setup(this, panes, style, name);};

  /// When (left) clicked, uses the GetSTC() for some dialogs.
  virtual void StatusBarClicked(const std::string& pane);

  /// Do something when statusbar is (right) clicked.
  virtual void StatusBarClickedRight(const std::string& ) {};

  /// Returns text on specified pane.
  /// Don't forget to call SetupStatusBar first.
  static std::string GetStatusText(const std::string& pane);
  
  /// Sets text on specified pane.
  /// Don't forget to call SetupStatusBar first.
  static bool StatusText(const std::string& text, const std::string& pane);
  
  /// Updates statusbar pane items pane with values from specified listview.
  static bool UpdateStatusBar(const wxListView* lv);
  
  /// Updates the specified statusbar pane with values from specified stc.
  static bool UpdateStatusBar(wxExSTC* stc, const std::string& pane);

  /// Are we closing?
  static bool IsClosing() {return m_IsClosing;};
#endif // wxUSE_STATUSBAR
protected:
#if wxUSE_STATUSBAR
  // Interface from wxFrame.
  virtual wxStatusBar* OnCreateStatusBar(int number,
    long style, wxWindowID id, const wxString& name) override;
#endif

#if wxUSE_STATUSBAR
  static inline wxExStatusBar* m_StatusBar = nullptr;
#endif
private:
  static inline bool m_IsClosing = false;
  
  bool m_IsCommand {false};
  wxWindow* m_FindFocus {nullptr};
  wxFindReplaceDialog* m_FindReplaceDialog {nullptr};
  wxMenuBar* m_MenuBar {nullptr};
};
#endif // wxUSE_GUI
