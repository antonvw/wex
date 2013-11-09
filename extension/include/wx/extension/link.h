////////////////////////////////////////////////////////////////////////////////
// Name:      link.h
// Purpose:   Declaration of class wxExLink
// Author:    Anton van Wezenbeek
// Copyright: (c) 2013 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXLINK_H
#define _EXLINK_H

#include <wx/filefn.h> // for wxPathList

#if wxUSE_GUI

class wxExSTC;

/// Offers a class holding info about a link.
class WXDLLIMPEXP_BASE wxExLink
{
public:
  /// Default constructor.
  wxExLink(wxExSTC* stc = NULL);
  
  /// Adds a possible base path to pathlist, present in the stc component.
  /// Returns false if basepath was not found in stc (or stc is NULL).
  bool AddBasePath();
  
  /// Gets a path from text, using pathlist if necessary.
  /// Returns empty string if no path could be found.
  const wxString GetPath(
    /// text containing a path somewhere
    const wxString& text,
    /// line number to be filled in
    int& line_no,
    /// column to be filled in
    int& column_no) const;
  
  /// Sets pathlist with info from config.
  /// If there is no config, pathlist will be empty.
  void SetFromConfig();
private:
  const wxString FindPath(const wxString& text) const;
  bool SetLink(
    wxString& text,
    int& line_no,
    int& column_no) const;
  
  wxPathList m_PathList;
  wxExSTC* m_STC;
};
#endif // wxUSE_GUI
#endif
