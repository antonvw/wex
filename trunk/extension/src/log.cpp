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

bool wxExLog(const wxString& text, const wxFileName& filename)
{
  return wxFile(
    filename.GetFullPath(),
    wxFile::write_append).Write(
      wxDateTime::Now().Format() + " " + text + wxTextFile::GetEOL());
}

const wxFileName wxExLogfileName()
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

bool wxExLog::SetLogging(bool logging) 
{
  if (logging)
  {
    if (!wxExLogfileName().FileExists())
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
