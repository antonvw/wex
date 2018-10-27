////////////////////////////////////////////////////////////////////////////////
// Name:      tool.cpp
// Purpose:   Implementation of wex::tool class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/tool.h>
#include <wex/statistics.h>

std::map < int, wex::tool_info > wex::tool::m_ToolInfo;

wex::tool::tool(int type)
  : m_Id(type)
{
  if (m_ToolInfo.empty())
  {
    AddInfo(ID_TOOL_REPORT_FIND, _("Found %d matches in").ToStdString());
    AddInfo(ID_TOOL_REPLACE, _("Replaced %d matches in").ToStdString());
    AddInfo(ID_TOOL_REPORT_KEYWORD, 
      _("Reported %d keywords in").ToStdString(), _("Report &Keyword").ToStdString());
  }
}

const std::string wex::tool::Info() const
{
  if (const auto& it = m_ToolInfo.find(m_Id); it != m_ToolInfo.end())
    return it->second.GetInfo();
  else 
    return std::string();
}

const std::string wex::tool::Info(const wex::statistics<int>* stat) const
{
  wxString logtext(Info());

  if (logtext.Contains("%d"))
  {
    logtext = logtext.Format(logtext, stat->Get(_("Actions Completed").ToStdString()));
  }

  logtext << " " << stat->Get(_("Files").ToStdString()) << " " << _("file(s)");

  return logtext.ToStdString();
}
