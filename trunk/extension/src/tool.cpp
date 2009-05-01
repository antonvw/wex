/******************************************************************************\
* File:          tool.cpp
* Purpose:       Implementation of wxExTool classes
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2008 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/extension/tool.h>
#include <wx/extension/app.h>
#include <wx/extension/config.h>
#include <wx/extension/util.h>

std::map < int, const wxExToolInfo > wxExTool::m_ToolInfo;

wxExTool::wxExTool(int type)
  : m_Id(type)
{
}

void wxExTool::AddInfo(
  int tool_id,
  const wxString& info,
  const wxString& text,
  bool is_basic,
  const wxString& helptext)
{
  std::map < int, const wxExToolInfo >::const_iterator it = m_ToolInfo.find(tool_id);

  if (it != m_ToolInfo.end())
  {
    wxLogError("Toolinfo already exists");
  }
  else
  {
    const wxExToolInfo ti(info, text, is_basic, helptext);
    m_ToolInfo.insert(std::make_pair(tool_id, ti));
  }
}

const wxString wxExTool::Info() const
{
  std::map < int, const wxExToolInfo >::const_iterator it = m_ToolInfo.find(m_Id);

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

void wxExTool::Initialize()
{
  // If you change these labels, don't forget to change in listview too
  // for title when checking in-out.
  if (!wxExApp::GetConfigBool("SVN"))
  {
    AddInfo(ID_TOOL_REVISION_RECENT, _("Recent revision from"));
    AddInfo(ID_TOOL_REPORT_REVISION, _("Reported %ld revisions in"), _("&Revision"), false);
  }

  AddInfo(ID_TOOL_LINE_CODE, _("Parsed code lines"));
  AddInfo(ID_TOOL_LINE_COMMENT, _("Parsed comment lines"));

  AddInfo(ID_TOOL_REPORT_COUNT, _("Counted"), _("&Count"), false);
  AddInfo(ID_TOOL_REPORT_FIND, _("Found %ld matches in"));
  AddInfo(ID_TOOL_REPORT_REPLACE, _("Replaced %ld matches in"));
  AddInfo(ID_TOOL_REPORT_KEYWORD, _("Reported %ld keywords in"), _("&Keyword"), false);
}
