////////////////////////////////////////////////////////////////////////////////
// Name:      link.h
// Purpose:   Declaration of class wxExLink
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <memory>
#include <string>
#include <wx/extension/path.h>

#if wxUSE_GUI

#define LINK_LINE_OPEN_URL          -1
#define LINK_LINE_OPEN_MIME         -2
#define LINK_LINE_OPEN_URL_AND_MIME -3

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
  
  /// Returns a path from text, using paths if necessary,
  /// returns empty path if no path could be found.
  const wxExPath GetPath(
    /// text containing a path somewhere
    const std::string& text,
    /// control data to be filled in Line from data, 
    /// you can use:
    /// - LINK_LINE_OPEN_URL 
    /// - LINK_LINE_OPEN_MIME 
    /// - LINK_LINE_OPEN_URL_AND_MIMTE 
    /// as line number. 
    /// Afterwards Line and Col from data are filled in if possible.
    wxExControlData& data) const;
  
  /// Sets paths with info from config.
  /// If there is no config, paths will be empty.
  void SetFromConfig();
private:
  const wxExPath FindPath(const std::string& text, const wxExControlData& data) const;
  bool SetLink(wxExPath& text, wxExControlData& data) const;
  
  std::unique_ptr<wxExPaths> m_Paths;
  wxExSTC* m_STC;
};
#endif // wxUSE_GUI
