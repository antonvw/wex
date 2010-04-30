/******************************************************************************\
* File:          tool.cpp
* Purpose:       Implementation of wxExTool class
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/stdpaths.h>
#include <wx/textfile.h>
#include <wx/extension/tool.h>
#include <wx/extension/frame.h>
#include <wx/extension/log.h>
#include <wx/extension/statistics.h>
#include <wx/extension/vcs.h>

wxExTool* wxExTool::m_Self = NULL;

wxExTool::wxExTool(int type)
  : m_Id(type)
{
}

wxExTool* wxExTool::Get(bool createOnDemand)
{
  if (m_Self == NULL && createOnDemand)
  {
    m_Self = new wxExTool(0);

    m_Self->AddInfo(ID_TOOL_REVISION_RECENT, _("Recent revision from"));
    m_Self->AddInfo(ID_TOOL_REPORT_REVISION, _("Reported %ld revisions in"), _("Report &Revision"));
    m_Self->AddInfo(ID_TOOL_REPORT_COUNT, _("Counted"), _("Report &Count"));
    m_Self->AddInfo(ID_TOOL_REPORT_FIND, _("Found %ld matches in"));
    m_Self->AddInfo(ID_TOOL_REPORT_REPLACE, _("Replaced %ld matches in"));
    m_Self->AddInfo(ID_TOOL_REPORT_KEYWORD, _("Reported %ld keywords in"), _("Report &Keyword"));
  }

  return m_Self;
}

const wxFileName wxExTool::GetLogfileName() const
{
  wxFileName filename(
#ifdef wxExUSE_PORTABLE
    wxPathOnly(wxStandardPaths::Get().GetExecutablePath())
#else
    wxStandardPaths::Get().GetUserDataDir()
#endif
    + wxFileName::GetPathSeparator() + _("statistics.log"));

  return filename;
}

const wxString wxExTool::Info() const
{
  const auto it = m_Self->m_ToolInfo.find(m_Id);

  if (it != m_Self->m_ToolInfo.end())
  {
    return it->second.GetInfo();
  }

  wxFAIL;

  return wxEmptyString;
}

void wxExTool::Log(
  const wxExStatistics<long>* stat, 
  const wxString& caption, 
  bool log_to_file) const
{
  wxString logtext(Info());

  if (logtext.Contains("%ld"))
  {
    logtext = logtext.Format(logtext, stat->Get(_("Actions Completed")));
  }

  logtext
    << " " << stat->Get(_("Files")) << " " << _("file(s)")
    << (!caption.empty() ? ": " + caption: "");

#if wxUSE_STATUSBAR
  wxExFrame::StatusText(logtext);
#endif

  if (log_to_file && stat->Get(_("Files")) != 0)
  {
    wxExLog::Get()->Log(logtext);

    if (IsCount())
    {
      wxString logtext;

      logtext
        << caption
        << stat->Get()
        << wxTextFile::GetEOL();

      wxExLog log(GetLogfileName());
      log.Log(logtext);
    }
  }
}

wxExTool* wxExTool::Set(wxExTool* tool)
{
  wxExTool* old = m_Self;
  m_Self = tool;
  return old;
}
