////////////////////////////////////////////////////////////////////////////////
// Name:      stat.cpp
// Purpose:   Implementation of wxExStat class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/stat.h>

wxExStat::wxExStat(const wxString& fullpath) 
{
  Sync(fullpath);
}

bool wxExStat::Sync() 
{
  if (m_FullPath.empty())
  {
    m_IsOk = false;
  }
  else
  {
#ifdef _MSC_VER
    m_IsOk = (stat(m_FullPath.c_str(), this) != -1);
#else
    m_IsOk = (::stat(m_FullPath.c_str(), this) != -1);
#endif
  }
  
  return m_IsOk;
}
