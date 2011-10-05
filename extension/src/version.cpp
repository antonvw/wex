////////////////////////////////////////////////////////////////////////////////
// Name:      version.cpp
// Purpose:   Implementation of version info
// Author:    Anton van Wezenbeek
// Created:   2011-08-13
// Copyright: (c) 2011 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/version.h>

const wxExVersionInfo wxExGetVersionInfo() 
{
  return wxExVersionInfo(
    "wxExtension", 
    2, 9, 3, 
    "wxExtension 2.9.3",
    "(c) 1998-2011, Anton van Wezenbeek. " + wxString(_("All rights reserved.")));
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
    
const wxString wxExVersionInfo::GetVersionString() const
{
  wxString str;
  
  str << GetMajor() << '.' << GetMinor();
  
  if ( GetMicro() )
    str << '.' << GetMicro();

  return str;
}
