////////////////////////////////////////////////////////////////////////////////
// Name:      frame.h
// Purpose:   Declaration of wex::frame class
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

class wxFindReplaceDialog;
class wxListView;

namespace wex
{
  class grid;
  class listview;
  class path;
  class path;
  class process;
  class stc;
  class vcs_entry;

  /// Offers a frame with easy statusbar methods, 
  /// find/replace, and allows for file dropping.
  /// Also helps in maintaining access to the base controls
  /// (grid, listview and STC).
  class frame : public wxFrame
  {
  public:
    /// Default constructor,
    frame(const window_data& data = window_data());

    /// Destructor.
    virtual ~frame();

    /// Returns a grid.
    virtual grid* GetGrid();

    /// Returns a listview.
    virtual listview* GetListView();

    /// Returns an STC.
    virtual stc* GetSTC();

    /// Returns true if file is opened in a window.
    virtual bool IsOpen(const path& filename);

    /// Called when an item dialog command event is triggered.
    virtual void OnCommandItemDialog(
      wxWindowID,
      const wxCommandEvent&) {};

    /// Default opens the file using GetSTC.
    /// Returns stc component opened, or nullptr.
    virtual stc* OpenFile(
      const path& filename,
      const stc_data& data = stc_data());

    /// Allows you to open a filename with info from vcs.
    /// Returns stc component opened, or nullptr.
    virtual stc* OpenFile(
      const path& filename,
      const vcs_entry& vcs,
      const stc_data& data = stc_data());
      
    /// Allows you to open a filename with specified contents.
    /// Returns stc component opened, or nullptr.
    virtual stc* OpenFile(
      const path& filename,
      const std::string& text,
      const stc_data& data = stc_data());
    
    /// Allows you to e.g. add debugging.
    /// Default returns nullptr.
    virtual process* Process(const std::string& command) {return nullptr;};
      
    /// Sets the find focus to specified window.
    void SetFindFocus(wxWindow* focus) {m_FindFocus = focus;};
    
    /// Override from base class.
    virtual void SetMenuBar(wxMenuBar* bar) override;
    
    /// Allows derived class to update file history.
    virtual void SetRecentFile(const path& path) {;};

    /// Sets up the status bar if you want to use StatusText.
    statusbar* SetupStatusBar(
      const std::vector<statusbarpane>& panes,
      long style = wxST_SIZEGRIP,
      const wxString& name = "statusBar") {
      return statusbar::Setup(this, panes, style, name);};

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
    static bool UpdateStatusBar(stc* stc, const std::string& pane);

    /// Are we closing?
    static bool IsClosing() {return m_IsClosing;};
  protected:
    // Interface from wxFrame.
    virtual wxStatusBar* OnCreateStatusBar(int number,
      long style, wxWindowID id, const wxString& name) override;

    static inline statusbar* m_StatusBar = nullptr;
  private:
    static inline bool m_IsClosing = false;
    
    bool m_IsCommand {false};
    wxWindow* m_FindFocus {nullptr};
    wxFindReplaceDialog* m_FindReplaceDialog {nullptr};
    wxMenuBar* m_MenuBar {nullptr};
  };
};
