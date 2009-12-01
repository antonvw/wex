/******************************************************************************\
* File:          dir.cpp
* Purpose:       Implementation of wxExDirWithListView and wxExDirTool classes
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/extension/report/dir.h>
#include <wx/extension/report/listitem.h>
#include <wx/extension/report/listview.h>
#include <wx/extension/report/textfile.h>

wxExDirWithListView::wxExDirWithListView(wxExListViewFile* listview,
  const wxString& fullpath, const wxString& filespec, int flags)
  : wxExDir(fullpath, filespec, flags)
  , m_ListView(listview)
{
}

void wxExDirWithListView::OnDir(const wxString& dir)
{
  if (wxConfigBase::Get()->ReadBool(_("Add folders"), true))
  {
    wxExListItemWithFileName(m_ListView, dir, GetFileSpec()).Insert();
  }
}

void wxExDirWithListView::OnFile(const wxString& file)
{
  wxExListItemWithFileName item(m_ListView, file, GetFileSpec());
  item.Insert();

  // Don't move next code into insert, as it itself inserts!
  if (m_ListView->GetType() == wxExListViewWithFrame::LIST_VERSION)
  {
    wxExListItemWithFileName item(m_ListView, m_ListView->GetItemCount() - 1);

    wxExTextFileWithListView report(item.m_Statistics, ID_TOOL_REVISION_RECENT);
    if (report.SetupTool(ID_TOOL_REVISION_RECENT))
    {
      report.RunTool();
       item.UpdateRevisionList(report.GetRCS());
    }
  }
}

wxExDirTool::wxExDirTool(const wxExTool& tool,
  const wxString& fullpath, const wxString& filespec, int flags)
  : wxExDir(fullpath, filespec, flags)
  , m_Statistics(fullpath)
  , m_Tool(tool)
{
}

void wxExDirTool::OnFile(const wxString& file)
{
  const wxExFileName filename(file);

  if (filename.GetStat().IsOk())
  {
    wxExTextFileWithListView report(filename, m_Tool);
    report.RunTool();
    m_Statistics += report.GetStatistics();
  }
}
