////////////////////////////////////////////////////////////////////////////////
// Name:      link.h
// Purpose:   Declaration of class wxExLink
// Author:    Anton van Wezenbeek
// Copyright: (c) 2012 Anton van Wezenbeek
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
  /// Constructor.
  wxExLink(wxExSTC* stc);
  
  /// Adds a possible base path to pathlist, present in the stc component.
  /// Returns false if basepath was not found in stc.
  bool AddBasePath();
  
  /// Gets a possible path from text, does not use pathlist
  /// to make an existing path of it, use GetPath
  /// to retrieve an existing path.
  const wxString FindPath(const wxString& text) const;
  
  /// Gets a path from text, using pathlist if necessary.
  /// Returns empty string if no path could be found.
  const wxString GetPath(const wxString& text) const;
  
  /// Sets pathlist with info from config.
  /// If there is no config, pathlist will be empty.
  void SetFromConfig();
private:
  wxPathList m_PathList;
  wxExSTC* m_STC;
};
#endif // wxUSE_GUI
#endif
