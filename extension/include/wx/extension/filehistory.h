////////////////////////////////////////////////////////////////////////////////
// Name:      filehistory.h
// Purpose:   Include file for wxExFileHistory class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
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
    /// number of files to use
    size_t maxFiles = 9, 
    /// base for menu id
    wxWindowID idBase = wxID_FILE1, 
    /// if key is empty string, files
    /// are loaded / saved to default keys, otherwise to specified key.
    const wxString& key = wxEmptyString);

  /// Adds a file to the file history list, if the object has a pointer to 
  /// an appropriate file menu. 
  virtual void AddFileToHistory(const wxString& file) override;
  
  /// Clears history.
  void Clear();

  /// Returns the file at this index (zero-based).
  virtual wxString GetHistoryFile(size_t index = 0) const override;

  /// Returns a vector of max recent opened files.
  std::vector<wxString> GetVector(size_t max) const;
  
  /// Shows popup menu.
  void PopupMenu(wxWindow* win, 
    int clear_id, const wxPoint& pos = wxDefaultPosition) const;
  
  /// Saves the file history into the config. 
  void Save();

  /// Adds a recent file submenu to specified menu,
  /// and sets the file history to use it.
  void UseMenu(wxWindowID id, wxMenu* menu);
private:
  const wxString m_Key;
};
