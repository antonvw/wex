////////////////////////////////////////////////////////////////////////////////
// Name:      file-history.h
// Purpose:   Include file for wex::file_history class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/core/path.h>
#include <wx/window.h>

namespace wex
{
class file_history_imp;
class menu;

/// Offers file history methods.
class file_history
{
public:
  /// Default constructor.
  /// Fills the file history with items available from the config.
  file_history(
    /// max number of files to use
    size_t max_files = 9,
    /// base for menu id
    wxWindowID id_base = wxID_FILE1,
    /// if key is empty string, files
    /// are loaded / saved to default keys, otherwise to specified key.
    const std::string& key = std::string());

  /// Destructor.
  ~file_history();

  /// Returns the path at this index (zero-based).
  const path operator[](size_t index) const;

  /// Appends a file (if file exists) to the file history list.
  /// Returns true if file is appended.
  bool append(const path& p);

  /// Clears history.
  void clear();

  /// Returns true if history is empty.
  bool empty() const;

  /// Returns baseid.
  wxWindowID get_base_id() const;

  /// Returns a vector of max recent opened files.
  std::vector<path> get_history_files(size_t max) const;

  /// Returns max files.
  size_t get_max_files() const;

  /// Shows popup menu containing all recent opened files.
  void popup_menu(
    /// window which will get the popup menu
    wxWindow* win,
    /// adds a clear menu items as well (if not -1)
    int clear_id = -1,
    /// position for popup menu
    const wxPoint& pos = wxDefaultPosition) const;

  /// Saves the file history into the config.
  void save();

  /// Returns number of items.
  size_t size() const;

  /// Adds a recent file submenu to specified menu,
  /// and sets the file history to use it.
  void use_menu(wxWindowID id, wex::menu* menu);

private:
  file_history_imp* m_history;
};
}; // namespace wex
