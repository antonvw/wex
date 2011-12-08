////////////////////////////////////////////////////////////////////////////////
// Name:      link.h
// Purpose:   Declaration of class wxExLink
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXLINK_H
#define _EXLINK_H

#include <wx/filefn.h>

#if wxUSE_GUI

class wxExSTC;

/// Offers a class holding info about a link.
class WXDLLIMPEXP_BASE wxExLink
{
public:
  /// Constructor.
  wxExLink(wxExSTC* stc);
  
  /// Adds a possible base path.
  void AddBasePath();
  
  /// Gets a path from line.
  const wxString GetPath(const wxString& line) const;
  
  /// Gets text at current position.
  const wxString GetTextAtCurrentPos() const;
  
  /// Sets link with info from config.
  void SetFromConfig();
private:
  wxPathList m_PathList;
  wxExSTC* m_STC;
};
#endif // wxUSE_GUI
#endif
