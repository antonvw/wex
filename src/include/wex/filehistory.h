////////////////////////////////////////////////////////////////////////////////
// Name:      file_history.h
// Purpose:   Include file for wex::file_history class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <vector>
#include <wx/window.h>

namespace wex
{
  class file_history_imp;
  class path;

  /// Offers file history methods.
  class file_history
  {
  public:
    /// Default constructor.
    file_history(
      /// number of files to use
      size_t maxFiles = 9, 
      /// base for menu id
      wxWindowID idBase = wxID_FILE1, 
      /// if key is empty string, files
      /// are loaded / saved to default keys, otherwise to specified key.
      const std::string& key = std::string());
    
    /// Destructor.
   ~file_history();

    /// Adds a file to the file history list.
    void add(const path& p);
    
    /// Clears history.
    void clear();
    
    /// Returns baseid.
    wxWindowID get_base_id() const;
    
    /// Returns the file at this index (zero-based).
    path get_history_file(size_t index = 0) const;

    /// Returns a vector of max recent opened files.
    std::vector<path> get_history_files(size_t max) const;
    
    /// Returns max files.
    int get_max_files() const;

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
    void use_menu(wxWindowID id, wxMenu* menu);
  private:
    file_history_imp* m_History;
    const std::string m_Key;
  };
};
