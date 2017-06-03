////////////////////////////////////////////////////////////////////////////////
// Name:      link.h
// Purpose:   Declaration of class wxExLink
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <memory>
#include <string>

#if wxUSE_GUI

class wxExControlData;
class wxExPaths;
class wxExSTC;

/// Offers a class holding info about a link.
class WXDLLIMPEXP_BASE wxExLink
{
public:
  /// Default constructor.
  wxExLink(wxExSTC* stc = nullptr);

  /// Destructor.
 ~wxExLink();
  
  /// Returns a path from text, using paths if necessary.
  /// Returns empty string if no path could be found.
  const std::string GetPath(
    /// text containing a path somewhere
    const std::string& text,
    /// control data to be filled in
    /// Line from data:
    /// - -1: look for browse, and browse file
    /// - -2: look for browse
    /// Afterwards Line and Col from data are filled in if possible.
    wxExControlData& data) const;
  
  /// Sets paths with info from config.
  /// If there is no config, paths will be empty.
  void SetFromConfig();
private:
  const std::string FindPath(const std::string& text, const wxExControlData& data) const;
  bool SetLink(
    std::string& text, wxExControlData& data) const;
  
  std::unique_ptr<wxExPaths> m_Paths;
  wxExSTC* m_STC;
};
#endif // wxUSE_GUI
