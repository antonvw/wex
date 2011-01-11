/******************************************************************************\
* File:          log.cpp
* Purpose:       Implementation of wxExLog class
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/app.h>
#include <wx/file.h>
#include <wx/stdpaths.h>
#include <wx/textfile.h>
#include <wx/extension/log.h>

wxExLog::wxExLog(const wxFileName& filename)
  : m_FileName(filename)
{
  if (!m_FileName.FileExists())
  {
    m_Logging = wxFile().Create(m_FileName.GetFullPath());
  }
  else
  {
    m_Logging = true;
  }
}

bool wxExLog::Log(const wxString& text, bool add_timestamp ) const
{
  if (!m_Logging) 
  {
    return false;
  }
  
  wxFile file(m_FileName.GetFullPath(), wxFile::write_append);

  return 
    file.IsOpened() &&
    file.Write((add_timestamp ? wxDateTime::Now().Format() + " ": "") 
      + text + wxTextFile::GetEOL());
}
