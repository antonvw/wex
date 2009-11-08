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
#include <wx/extension/svn.h>

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

    if (!wxExSVN::Get()->Use())
    {
      m_Self->AddInfo(ID_TOOL_REVISION_RECENT, _("Recent revision from"));
      m_Self->AddInfo(ID_TOOL_REPORT_REVISION, _("Reported %ld revisions in"), _("Report &Revision"));
    }

    m_Self->AddInfo(ID_TOOL_LINE_CODE, _("Parsed code lines"));
    m_Self->AddInfo(ID_TOOL_LINE_COMMENT, _("Parsed comment lines"));

    m_Self->AddInfo(ID_TOOL_REPORT_COUNT, _("Counted"), _("Report &Count"));
    m_Self->AddInfo(ID_TOOL_REPORT_FIND, _("Found %ld matches in"));
    m_Self->AddInfo(ID_TOOL_REPORT_REPLACE, _("Replaced %ld matches in"));
    m_Self->AddInfo(ID_TOOL_REPORT_KEYWORD, _("Reported %ld keywords in"), _("Report &Keyword"));
  }

  return m_Self;
}

const wxString wxExTool::Info() const
{
  std::map < int, wxExToolInfo >::const_iterator it = m_Self->m_ToolInfo.find(m_Id);

  if (it != m_Self->m_ToolInfo.end())
  {
    return it->second.GetInfo();
  }

  wxFAIL;

  return wxEmptyString;
}

wxExTool* wxExTool::Set(wxExTool* tool)
{
  wxExTool* old = m_Self;
  m_Self = tool;
  return old;
}
