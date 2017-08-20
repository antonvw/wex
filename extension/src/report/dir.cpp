////////////////////////////////////////////////////////////////////////////////
// Name:      dir.cpp
// Purpose:   Implementation of wxExDirWithListView and wxExDirTool classes
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/extension/listitem.h>
#include <wx/extension/report/dir.h>
#include <wx/extension/report/stream.h>

wxExDirTool::wxExDirTool(const wxExTool& tool,
  const wxExPath& fullpath, const std::string& filespec, int flags)
  : wxExDir(fullpath, filespec, flags)
  , m_Statistics()
  , m_Tool(tool)
{
}

bool wxExDirTool::OnFile(const wxExPath& file)
{
  wxExStreamToListView report(file, m_Tool);

  bool ret = report.RunTool();
  m_Statistics += report.GetStatistics();

  if (!ret)
  {
    Cancel();
    return false;
  }

  return true;
}

wxExDirWithListView::wxExDirWithListView(wxExListView* listview,
  const wxExPath& fullpath, const std::string& filespec, int flags)
  : wxExDir(fullpath, filespec, flags)
  , m_ListView(listview)
{
}

bool wxExDirWithListView::OnDir(const wxExPath& dir)
{
  wxExListItem(m_ListView, dir, GetFileSpec()).Insert();
  return true;
}

bool wxExDirWithListView::OnFile(const wxExPath& file)
{
  wxExListItem(m_ListView, file, GetFileSpec()).Insert();
  return true;
}
