////////////////////////////////////////////////////////////////////////////////
// Name:      frame.h
// Purpose:   Declaration of wex::frame class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <vector>
#include <wx/frame.h>
#include <wex/statusbar.h>
#include <wex/stc-data.h>
#include <wex/window-data.h>

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
  /// (grid, listview and stc).
  class frame : public wxFrame
  {
  public:
    /// Default constructor,
    frame(const window_data& data = window_data());

    /// Destructor.
    virtual ~frame();

    /// Override from base class.
    virtual void SetMenuBar(wxMenuBar* bar) override;
    
    /// Virtual interface
    
    /// Returns a grid.
    virtual grid* get_grid();

    /// Returns a listview.
    virtual listview* get_listview();

    /// Allows you to e.g. add debugging.
    /// Default returns nullptr.
    virtual process* get_process(const std::string& command) {return nullptr;};
      
    /// Returns an stc.
    virtual stc* get_stc();

    /// Returns true if file is opened in a window.
    virtual bool is_open(const path& filename);

    /// Called when an item dialog command event is triggered.
    virtual void on_command_item_dialog(
      wxWindowID,
      const wxCommandEvent&) {};

    /// Default opens the file using get_stc.
    /// Returns stc component opened, or nullptr.
    virtual stc* open_file(
      const path& filename,
      const stc_data& data = stc_data());

    /// Allows you to open a filename with info from vcs.
    /// Returns stc component opened, or nullptr.
    virtual stc* open_file(
      const path& filename,
      const vcs_entry& vcs,
      const stc_data& data = stc_data());
      
    /// Allows you to open a filename with specified contents.
    /// Returns stc component opened, or nullptr.
    virtual stc* open_file(
      const path& filename,
      const std::string& text,
      const stc_data& data = stc_data());
    
    /// Allows derived class to update file history.
    virtual void set_recent_file(const path& path) {;};

    /// When (left) clicked, uses the get_stc() for some dialogs.
    virtual void statusbar_clicked(const std::string& pane);

    /// Do something when statusbar is (right) clicked.
    virtual void statusbar_clicked_right(const std::string& ) {};
    
    /// Other methods

    /// Sets the find focus to specified window.
    void set_find_focus(wxWindow* focus) {m_FindFocus = focus;};
    
    /// Sets up the status bar if you want to use statustext.
    statusbar* setup_statusbar(
      const std::vector<statusbar_pane>& panes,
      long style = wxST_SIZEGRIP,
      const wxString& name = "statusBar") {
      return statusbar::setup(this, panes, style, name);};

    /// Returns text on specified pane.
    /// Don't forget to call setup_statusbar first.
    static std::string get_statustext(const std::string& pane);
    
    /// Are we closing?
    static bool is_closing() {return m_is_closing;};

    /// Sets text on specified pane.
    /// Don't forget to call setup_statusbar first.
    static bool statustext(const std::string& text, const std::string& pane);
    
    /// Updates statusbar pane items pane with values from specified listview.
    static bool update_statusbar(const wxListView* lv);
    
    /// Updates the specified statusbar pane with values from specified stc.
    static bool update_statusbar(stc* stc, const std::string& pane);
  protected:
    // Interface from wxFrame.
    virtual wxStatusBar* OnCreateStatusBar(int number,
      long style, wxWindowID id, const wxString& name) override;

    static inline statusbar* m_StatusBar = nullptr;
  private:
    static inline bool m_is_closing = false;
    
    bool m_IsCommand {false};
    wxWindow* m_FindFocus {nullptr};
    wxFindReplaceDialog* m_FindReplaceDialog {nullptr};
    wxMenuBar* m_MenuBar {nullptr};
  };
};
