////////////////////////////////////////////////////////////////////////////////
// Name:      dir.cpp
// Purpose:   Implementation of wxExDirWithListView and wxExDirTool classes
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/extension/listitem.h>
#include <wx/extension/report/dir.h>

wxExDirTool::wxExDirTool(const wxExTool& tool,
  const wxString& fullpath, const wxString& filespec, int flags)
  : wxExDir(fullpath, filespec, flags)
  , m_Statistics()
  , m_Tool(tool)
{
}

void wxExDirTool::OnFile(const wxString& file)
{
  wxExTextFileWithListView report(file, m_Tool);
  report.RunTool();
  m_Statistics += report.GetStatistics();
}

wxExDirWithListView::wxExDirWithListView(wxExListView* listview,
  const wxString& fullpath, const wxString& filespec, int flags)
  : wxExDir(fullpath, filespec, flags)
  , m_ListView(listview)
{
}

void wxExDirWithListView::OnDir(const wxString& dir)
{
  if (wxConfigBase::Get()->ReadBool(_("Add folders"), true))
  {
    wxExListItem(m_ListView, dir, GetFileSpec()).Insert();
  }
}

void wxExDirWithListView::OnFile(const wxString& file)
{
  wxExListItem(m_ListView, file, GetFileSpec()).Insert();
}
