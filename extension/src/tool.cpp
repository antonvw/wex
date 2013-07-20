////////////////////////////////////////////////////////////////////////////////
// Name:      tool.cpp
// Purpose:   Implementation of wxExTool class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2013 Anton van Wezenbeek
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
    AddInfo(ID_TOOL_REPORT_FIND, _("Found %d matches in"));
    AddInfo(ID_TOOL_REPORT_REPLACE, _("Replaced %d matches in"));
    AddInfo(ID_TOOL_REPORT_KEYWORD, _("Reported %d keywords in"), _("Report &Keyword"));
  }
}

const wxString wxExTool::Info() const
{
  const auto it = m_ToolInfo.find(m_Id);

  if (it != m_ToolInfo.end())
  {
    return it->second.GetInfo();
  }

  return wxEmptyString;
}

const wxString wxExTool::Info(const wxExStatistics<int>* stat) const
{
  wxString logtext(Info());

  if (logtext.Contains("%d"))
  {
    logtext = logtext.Format(logtext, stat->Get(_("Actions Completed")));
  }

  logtext << " " << stat->Get(_("Files")) << " " << _("file(s)");

  return logtext;
}
