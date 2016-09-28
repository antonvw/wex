////////////////////////////////////////////////////////////////////////////////
// Name:      link.h
// Purpose:   Declaration of class wxExLink
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/filefn.h> // for wxPathList

#if wxUSE_GUI

class wxExSTC;

/// Offers a class holding info about a link.
class WXDLLIMPEXP_BASE wxExLink
{
public:
  /// Default constructor.
  wxExLink(wxExSTC* stc = nullptr);
  
  /// Returns a path from text, using pathlist if necessary.
  /// Returns empty string if no path could be found.
  const std::string GetPath(
    /// text containing a path somewhere
    const std::string& text,
    /// line number to be filled in
    int& line_no,
    /// column to be filled in
    int& column_no) const;
  
  /// Sets pathlist with info from config.
  /// If there is no config, pathlist will be empty.
  void SetFromConfig();
private:
  const std::string FindPath(const std::string& text, int line_no) const;
  bool SetLink(
    std::string& text,
    int& line_no,
    int& column_no) const;
  
  wxPathList m_PathList;
  wxExSTC* m_STC;
};
#endif // wxUSE_GUI
