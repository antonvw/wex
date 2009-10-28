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

#include <wx/app.h>
#include <wx/file.h>
#include <wx/stdpaths.h>
#include <wx/textfile.h>
#include <wx/extension/log.h>

wxExLog* wxExLog::m_Self = NULL;

wxExLog::wxExLog(const wxFileName& filename)
  : m_Logging(true)
  , m_FileName(filename)
{
}

void wxExLog::Destroy()
{
  delete m_Self;
}

wxExLog* wxExLog::Get(bool createOnDemand)
{
  if (m_Self == NULL)
  {
    wxFileName filename;

    if (wxTheApp == NULL)
    {
      filename = wxFileName("app.log");
    }

#ifdef wxExUSE_PORTABLE
    filename = wxFileName(
      wxPathOnly(wxStandardPaths::Get().GetExecutablePath()),
      wxTheApp->GetAppName().Lower() + ".log");
#else
    filename = wxFileName(
      wxStandardPaths::Get().GetUserDataDir(),
      wxTheApp->GetAppName().Lower() + ".log");
#endif

    m_Self = new wxExLog(filename);
    m_Self->m_Logging = false;
  }

  return m_Self;
}

bool wxExLog::Log(const wxString& text) 
{
  if (m_Logging) 
  {
    return 
      wxFile(GetFileName().GetFullPath(), wxFile::write_append).Write(
        wxDateTime::Now().Format() + " " + text + wxTextFile::GetEOL());
  }
  else
  {
    return false;
  }
}

bool wxExLog::SetLogging(bool logging) 
{
  if (logging)
  {
    if (!GetFileName().FileExists())
    {
      m_Logging = wxFile().Create(GetFileName().GetFullPath());
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
