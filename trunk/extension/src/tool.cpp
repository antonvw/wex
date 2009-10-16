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

#include <wx/config.h>
#include <wx/extension/tool.h>

std::map < int, wxExToolInfo > wxExTool::m_ToolInfo;

wxExTool::wxExTool(int type)
  : m_Id(type)
{
}

const wxString wxExTool::Info() const
{
  std::map < int, wxExToolInfo >::const_iterator it = m_ToolInfo.find(m_Id);

  if (it != m_ToolInfo.end())
  {
    return it->second.GetInfo();
  }

  wxFAIL;

  return wxEmptyString;
}

void wxExTool::Initialize()
{
  if (!wxConfigBase::Get()->ReadBool("SVN", true))
  {
    AddInfo(ID_TOOL_REVISION_RECENT, _("Recent revision from"));
    AddInfo(ID_TOOL_REPORT_REVISION, _("Reported %ld revisions in"), _("Report &Revision"));
  }

  AddInfo(ID_TOOL_LINE_CODE, _("Parsed code lines"));
  AddInfo(ID_TOOL_LINE_COMMENT, _("Parsed comment lines"));

  AddInfo(ID_TOOL_REPORT_COUNT, _("Counted"), _("Report &Count"));
  AddInfo(ID_TOOL_REPORT_FIND, _("Found %ld matches in"));
  AddInfo(ID_TOOL_REPORT_REPLACE, _("Replaced %ld matches in"));
  AddInfo(ID_TOOL_REPORT_KEYWORD, _("Reported %ld keywords in"), _("Report &Keyword"));
}
