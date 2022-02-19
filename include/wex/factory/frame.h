////////////////////////////////////////////////////////////////////////////////
// Name:      factory/frame.h
// Purpose:   Declaration of wex::factory::frame class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/frame.h>

class wxListView;

namespace wex
{
class path;
class vcs_entry;

namespace data
{
class stc;
};

namespace factory
{
class grid;
class listview;
class process;
class stc;

/// Offers a frame with easy statusbar methods,
/// find/replace, and allows for file dropping.
/// Also helps in maintaining access to the base controls
/// (grid, listview and stc).
class frame : public wxFrame
{
public:
  /// Destructor.
  virtual ~frame() = default;

  /// Virtual interface

  /// Returns a grid.
  virtual factory::grid* get_grid();

  /// Returns a listview.
  virtual factory::listview* get_listview();

  /// Allows you to e.g. add debugging.
  /// Default returns nullptr.
  virtual factory::process* get_process(const std::string& command)
  {
    return nullptr;
  };

  /// Returns text on specified pane.
  /// Don't forget to call setup_statusbar first.
  virtual std::string get_statustext(const std::string& pane) const
  {
    return std::string();
  };

  /// Returns an stc.
  virtual factory::stc* get_stc();

  /// Returns true if file is opened in a window.
  virtual bool is_open(const path& filename);

  /// Called when an item dialog command event is triggered.
  virtual void on_command_item_dialog(wxWindowID, const wxCommandEvent&) {}

  /// Default opens the file using get_stc.
  /// Returns stc component opened, or nullptr.
  virtual factory::stc* open_file(const path& filename, const data::stc& data);

  /// Allows you to open a filename with specified contents.
  /// Returns stc component opened, or nullptr.
  virtual factory::stc* open_file(
    const path&        filename,
    const std::string& text,
    const data::stc&   data);

  /// Allows you to open a filename with info from vcs.
  /// Returns stc component opened, or nullptr.
  virtual factory::stc*
  open_file(const path& filename, const vcs_entry& vcs, const data::stc& data)
  {
    return nullptr;
  };

  /// Allows you to handle output text, .e.g. from a process.
  virtual bool output(const std::string& text) const { return false; }

  /// Allows derived class to update file history.
  virtual void set_recent_file(const path& path) { ; }

  /// When (left) clicked, uses the get_stc() for some dialogs.
  virtual void statusbar_clicked(const std::string& pane);

  /// Do something when statusbar is (right) clicked.
  virtual void statusbar_clicked_right(const std::string&) {}

  /// Sets text on specified pane.
  /// Don't forget to call setup_statusbar first.
  virtual bool
  statustext(const std::string& text, const std::string& pane) const
  {
    return false;
  };

  /// Other methods

  /// Are we closing?
  bool is_closing() const { return m_is_closing; }

  /// Sets the find focus to specified window.
  void set_find_focus(wxWindow* focus) { m_find_focus = focus; }

  /// Updates statusbar pane items pane with values from specified listview.
  bool update_statusbar(const wxListView* lv);

  /// Updates the specified statusbar pane with values from specified stc.
  bool update_statusbar(factory::stc* stc, const std::string& pane);

  /// Override from base class.

  void SetMenuBar(wxMenuBar* bar) override;

protected:
  wxWindow* m_find_focus{nullptr};

  bool m_is_closing{false};
  bool m_is_command{false};

  wxMenuBar* m_menubar{nullptr};
};
}; // namespace factory
} // namespace wex
