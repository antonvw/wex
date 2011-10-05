////////////////////////////////////////////////////////////////////////////////
// Name:      version.h
// Purpose:   Declaration of version macro
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/versioninfo.h>

/// This class offers our own version info.
class WXDLLIMPEXP_BASE wxExVersionInfo : public wxVersionInfo
{
public:
  /// Constructor.
  wxExVersionInfo(const wxString& name = wxString(),
    int major = 0,
    int minor = 0,
    int micro = 0,
    const wxString& description = wxString(),
    const wxString& copyright = wxString());
                  
  /// Override base, this one does not include name.
  const wxString GetVersionString() const;
};

const wxExVersionInfo wxExGetVersionInfo();
