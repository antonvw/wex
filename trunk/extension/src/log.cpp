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

wxExLog* wxExLog::m_Self = NULL;

wxExLog::wxExLog(const wxFileName& filename, bool logging)
  : m_FileName(filename)
{
  SetLogging(logging);
}

wxExLog* wxExLog::Get(bool createOnDemand)
{
  if (m_Self == NULL && createOnDemand)
  {
    wxFileName filename;

    if (wxTheApp == NULL)
    {
      filename = wxFileName("app.log");
    }
    else
    {
#ifdef wxExUSE_PORTABLE
      filename = wxFileName(
        wxPathOnly(wxStandardPaths::Get().GetExecutablePath()),
        wxTheApp->GetAppName().Lower() + ".log");
#else
      filename = wxFileName(
        wxStandardPaths::Get().GetUserDataDir(),
        wxTheApp->GetAppName().Lower() + ".log");
#endif
    }

    m_Self = new wxExLog(filename, false); // no logging
  }

  return m_Self;
}

bool wxExLog::Log(const wxString& text, bool add_timestamp ) const
{
  if (m_Logging) 
  {
    wxFile file(m_FileName.GetFullPath(), wxFile::write_append);

    const wxString log = (add_timestamp ? wxDateTime::Now().Format() + " ": "") 
      + text + wxTextFile::GetEOL();

    return file.Write(log);
  }

  return false;
}

wxExLog* wxExLog::Set(wxExLog* log)
{
  wxExLog* old = m_Self;
  m_Self = log;
  return old;
}

void wxExLog::SetLogging(bool logging) 
{
  if (logging)
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
  else
  {
    m_Logging = false;
  }
}
