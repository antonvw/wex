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

const wxVersionInfo wxExGetVersionInfo() 
{
  return wxVersionInfo("wxExtension", 1, 1, 0, "wxExtension 1.1.0");
}
