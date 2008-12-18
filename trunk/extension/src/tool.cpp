/******************************************************************************\
* File:          tool.cpp
* Purpose:       Implementation of exTool classes
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2008 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/extension/extension.h>
#include <wx/extension/tool.h>

std::map < int, const exToolInfo > exTool::m_ToolInfo;

exTool::exTool(int type)
  : m_Id(type)
{
}

void exTool::AddInfo(
  int tool_id,
  const wxString& info,
  const wxString& text,
  bool is_basic,
  const wxString& helptext)
{
  std::map < int, const exToolInfo >::const_iterator it = m_ToolInfo.find(tool_id);

  if (it != m_ToolInfo.end())
  {
    wxLogError("Toolinfo already exists");
  }
  else
  {
    const exToolInfo ti(info, text, is_basic, helptext);
    m_ToolInfo.insert(std::make_pair(tool_id, ti));
  }
}

const wxString exTool::Info() const
{
  if (m_ToolInfo.empty())
  {
    wxLogError("Toolinfo is empty");
    return wxEmptyString;
  }

  std::map < int, const exToolInfo >::const_iterator it = m_ToolInfo.find(m_Id);

  if (it != m_ToolInfo.end())
  {
    return it->second.GetInfo();
  }
  else
  {
    wxLogError("Could not find tool: %d", m_Id);
    return wxEmptyString;
  }
}

void exTool::Initialize()
{
  // If you change these labels, don't forget to change in listview too 
  // for title when checking in-out.
  if (exApp::GetConfigBool("RCS/Local"))
  {
    AddInfo(ID_TOOL_COMMIT, _("Commited"), exEllipsed(_("&Commit")));
    AddInfo(ID_TOOL_REVISION_RECENT, _("Recent revision from"));
    AddInfo(ID_TOOL_REPORT_REVISION, _("Reported %ld revisions in"), _("&Revision"), false);
  }

  AddInfo(ID_TOOL_HEADER, _("Edited header in"), exEllipsed(_("&Header")));
  AddInfo(ID_TOOL_LINE_CODE, _("Parsed code lines"));
  AddInfo(ID_TOOL_LINE_COMMENT, _("Parsed comment lines"));

  AddInfo(ID_TOOL_REPORT_COUNT, _("Counted"), _("&Count"), false);
  AddInfo(ID_TOOL_REPORT_FIND, _("Found %ld matches in"));
  AddInfo(ID_TOOL_REPORT_REPLACE, _("Replaced %ld matches in"));
  AddInfo(ID_TOOL_REPORT_HEADER, _("Reported %ld headers in"), _("&Header"), false);
  AddInfo(ID_TOOL_REPORT_KEYWORD, _("Reported %ld keywords in"), _("&Keyword"), false);
}
