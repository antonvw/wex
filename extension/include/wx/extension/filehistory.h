////////////////////////////////////////////////////////////////////////////////
// Name:      filehistory.h
// Purpose:   Include file for wxExFileHistory class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <vector>
#include <wx/window.h>

class wxExFileHistoryImp;

/// Offers some extra methods to wxFileHistory.
class WXDLLIMPEXP_BASE wxExFileHistory
{
public:
  /// Default constructor.
  wxExFileHistory(
    /// number of files to use
    size_t maxFiles = 9, 
    /// base for menu id
    wxWindowID idBase = wxID_FILE1, 
    /// if key is empty string, files
    /// are loaded / saved to default keys, otherwise to specified key.
    const std::string& key = std::string());
  
  /// Destructor.
 ~wxExFileHistory();

  /// Adds a file to the file history list, if the object has a pointer to 
  /// an appropriate file menu. 
  void AddFileToHistory(const std::string& file);
  
  /// Clears history.
  void Clear();
  
  /// Returns baseid.
  wxWindowID GetBaseId() const;
  
  /// Returns number of items.
  size_t GetCount() const;
  
  /// Returns the file at this index (zero-based).
  std::string GetHistoryFile(size_t index = 0) const;

  /// Returns a vector of max recent opened files.
  std::vector<std::string> GetHistoryFiles(size_t max) const;
  
  /// Returns max files.
  int GetMaxFiles() const;

  /// Shows popup menu containing all recent opened files.
  void PopupMenu(
    /// window which will get the popup menu
    wxWindow* win, 
    /// adds a clear menu items as well (if not -1)
    int clear_id = -1, 
    /// position for popup menu
    const wxPoint& pos = wxDefaultPosition) const;
  
  /// Saves the file history into the config. 
  void Save();

  /// Adds a recent file submenu to specified menu,
  /// and sets the file history to use it.
  void UseMenu(wxWindowID id, wxMenu* menu);
private:
  wxExFileHistoryImp* m_History;
  const std::string m_Key;
};
