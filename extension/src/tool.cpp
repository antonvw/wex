////////////////////////////////////////////////////////////////////////////////
// Name:      tool.cpp
// Purpose:   Implementation of wxExTool class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/tool.h>
#include <wx/extension/statistics.h>

std::map < int, wxExToolInfo > wxExTool::m_ToolInfo;

wxExTool::wxExTool(int type)
  : m_Id(type)
{
  if (m_ToolInfo.empty())
  {
    AddInfo(ID_TOOL_REVISION_RECENT, _("Recent revision from"));
    AddInfo(ID_TOOL_REPORT_REVISION, _("Reported %ld revisions in"), _("Report &Revision"));
    AddInfo(ID_TOOL_REPORT_COUNT, _("Counted"), _("Report &Count"));
    AddInfo(ID_TOOL_REPORT_FIND, _("Found %ld matches in"));
    AddInfo(ID_TOOL_REPORT_REPLACE, _("Replaced %ld matches in"));
    AddInfo(ID_TOOL_REPORT_KEYWORD, _("Reported %ld keywords in"), _("Report &Keyword"));
  }
}

const wxString wxExTool::Info() const
{
#ifdef wxExUSE_CPP0X	
  const auto it = m_ToolInfo.find(m_Id);
#else
  std::map < int, wxExToolInfo >::const_iterator it = m_ToolInfo.find(m_Id);
#endif  

  if (it != m_ToolInfo.end())
  {
    return it->second.GetInfo();
  }

  return wxEmptyString;
}

void wxExTool::Log(const wxExStatistics<long>* stat) const
{
  wxString logtext(Info());

  if (logtext.Contains("%ld"))
  {
    logtext = logtext.Format(logtext, stat->Get(_("Actions Completed")));
  }

  logtext << " " << stat->Get(_("Files")) << " " << _("file(s)");

  if (stat->Get(_("Files")) != 0)
  {
    if (IsCount())
    {
      wxLogMessage(stat->Get());
    }
  }
  
  wxLogStatus(logtext);
}
