/******************************************************************************\
* File:          statistics.cpp
* Purpose:       Implementation of statistics classes
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2008 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/stdpaths.h>
#include <wx/textfile.h>
#include <wx/extension/statistics.h>
#include <wx/extension/app.h>
#include <wx/extension/util.h>

long wxExFileNameStatistics::Get(const wxString& key) const
{
  std::map<wxString, long>::const_iterator it = m_Elements.GetItems().find(key);

  if (it != m_Elements.GetItems().end())
  {
    return it->second;
  }
  else
  {
    std::map<wxString, long>::const_iterator it = m_Keywords.GetItems().find(key);

    if (it != m_Keywords.GetItems().end())
    {
      return it->second;
    }
  }

  return 0;
}

const wxFileName wxExFileNameStatistics::GetLogfileName()
{
  wxFileName filename(
#ifdef EX_PORTABLE
    wxPathOnly(wxStandardPaths::Get().GetExecutablePath())
#else
    wxStandardPaths::Get().GetUserDataDir()
#endif
    + GetPathSeparator() + _("statistics.log"));

  return filename;
}

void wxExFileNameStatistics::Log(
  const wxExTool& tool,
  bool log_to_file) const
{
  // This is no error, if you run a tool and you cancelled everything,
  // the elements will be empty, so just quit.
  if (m_Elements.GetItems().empty())
  {
    return;
  }

  wxString logtext(tool.Info());

  if (logtext.Contains("%ld"))
  {
    logtext = logtext.Format(logtext, Get(_("Actions Completed")));
  }

  logtext
    << " " << Get(_("Files Passed")) << " " << _("file(s)")
    << (IsOk() ? ": " + GetFullPath(): "");

  wxExFrame::StatusText(logtext);

  if (log_to_file && Get(_("Files Passed")) != 0)
  {
    wxExApp::Log(logtext);

    if (tool.IsCountType())
    {
      wxString logtext;

      logtext
        << GetFullPath()
        << m_Elements.Get()
        << wxTextFile::GetEOL();

      wxExLog(logtext, GetLogfileName());
    }
  }
}
