/******************************************************************************\
* File:          app.cpp
* Purpose:       Implementation of wxExApp class
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/file.h>
#include <wx/extension/log.h>

bool wxExLog::m_Logging = false;

const wxFileName wxExLog::GetFileName()
{
  if (wxTheApp == NULL)
  {
    return wxFileName("app.log");
  }

#ifdef wxExUSE_PORTABLE
  return wxFileName(
    wxPathOnly(wxStandardPaths::Get().GetExecutablePath()),
    wxTheApp->GetAppName().Lower() + ".log");
#else
  return wxFileName(
    wxStandardPaths::Get().GetUserDataDir(),
    wxTheApp->GetAppName().Lower() + ".log");
#endif
}

bool wxExLog::Log(const wxString& text) 
{
  if (m_Logging) return Log(text, GetFileName());
  else           return false;
}

bool wxExLog::Log(const wxString& text, const wxFileName& filename)
{
  return wxFile(
    filename.GetFullPath(),
    wxFile::write_append).Write(
      wxDateTime::Now().Format() + " " + text + wxTextFile::GetEOL());
}

bool wxExLog::SetLogging(bool logging) 
{
  if (logging)
  {
    if (!GetFileName().FileExists())
    {
      m_Logging = wxFile().Create(wxExLogfileName().GetFullPath());
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

  return m_Logging;
}
