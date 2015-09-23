////////////////////////////////////////////////////////////////////////////////
// Name:      filehistory.h
// Purpose:   Include file for wxExFileHistory class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <vector>
#include <wx/filehistory.h>
#include <wx/window.h>

/// Offers some extra methods to wxFileHistory.
class WXDLLIMPEXP_BASE wxExFileHistory : public wxFileHistory
{
public:
  /// Default constructor.
  wxExFileHistory(
    // number of files to use
    size_t maxFiles = 9, 
    // base for menu id
    wxWindowID idBase = wxID_FILE1, 
    /// if key is empty string, files
    /// are loaded / saved to default keys, otherwise to specified key.
    const wxString& key = wxEmptyString);

  /// Clears history.
  void Clear();

  /// Returns the recent opened file.
  const wxString GetRecentFile(size_t index = 0);

  /// Returns a vector of max recent opened files.
  std::vector<wxString> GetVector(size_t max);
  
  /// Shows popup menu.
  void PopupMenu(wxWindow* win, 
    int clear_id, const wxPoint& pos = wxDefaultPosition) const;
  
  /// Saves the recent opened files to config. 
  void Save();

  /// Sets recent opened file.
  bool SetRecentFile(const wxString& file);
  
  /// Adds a recent file menu to specified menu,
  /// and sets the file history to use it.
  void UseMenu(wxWindowID id, wxMenu* menu);
private:
  const wxString m_Key;
};
