////////////////////////////////////////////////////////////////////////////////
// Name:      version.cpp
// Purpose:   Implementation of version info
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/version.h>

const wxExVersionInfo wxExGetVersionInfo() 
{
  return wxExVersionInfo("wxExtension", 
    17, 10, 0, 
    "wxExtension Library (a collection of wxWidgets extension classes)",
    "(c) 1998-2017, Anton van Wezenbeek. " + wxString(_("All rights reserved.")));
}

wxExVersionInfo::wxExVersionInfo(const wxString& name,
  int major,
  int minor,
  int micro,
  const wxString& description,
  const wxString& copyright)
  : wxVersionInfo(name, major, minor, micro, description, copyright)
{
}
    
const wxString wxExVersionInfo::GetVersionOnlyString() const
{
  return wxString::Format("%d.%02d.%d", GetMajor(), GetMinor(), GetMicro());
}
