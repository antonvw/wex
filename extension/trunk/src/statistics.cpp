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
#include <wx/extension/extension.h>
#include <wx/extension/statistics.h>
#include <wx/extension/stc.h>
#include <wx/extension/textfile.h>

long exFileNameStatistics::Get(const wxString& key) const
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

const wxFileName exFileNameStatistics::GetLogfileName()
{
  wxFileName filename(
#ifdef EX_PORTABLE
  wxPathOnly(wxStandardPaths::Get().GetExecutablePath()),
#else
  wxStandardPaths::Get().GetUserDataDir(),
#endif
  _("statistics.log"));

  return filename;
}

void exFileNameStatistics::Log(
  bool log_to_file,
  bool open_file) const
{
  // This is no error, if you run a tool and you cancelled everything,
  // the elements will be empty, so just quit.
  if (m_Elements.GetItems().empty())
  {
    return;
  }

  wxString logtext(exTextFile::GetTool().Info());

  if (logtext.Contains("%ld"))
  {
    logtext = logtext.Format(logtext, Get(_("Actions Completed")));
  }

  logtext 
    << " " << Get(_("Files Passed")) << " " << _("file(s)")
    << (IsOk() ? ": " + GetFullPath(): "");

  exFrame::StatusText(logtext);

  if (!log_to_file) return;

  if (Get(_("Files Passed")) != 0)
  {
    exApp::Log(logtext);

    if (exTextFile::GetTool().IsCountType())
    {
      wxString logtext;

      logtext 
        << GetFullPath()
        << m_Elements.Get() 
        << wxTextFile::GetEOL();

      exLog(logtext, GetLogfileName());

      if (open_file)
      {
        exOpenFile(GetLogfileName(), exSTC::STC_OPEN_FROM_STATISTICS);
      }
    }
  }
}
