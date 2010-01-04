/******************************************************************************\
* File:          listitem.cpp
* Purpose:       Implementation of class 'wxExListItemWithFileName'
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/config.h>
#include <wx/extension/report/listitem.h>
#include <wx/extension/frame.h>
#include <wx/extension/report/dir.h>
#include <wx/extension/report/textfile.h>
#include <wx/extension/report/util.h>

// Do not give an error if columns do not exist.
// E.g. the LIST_PROCESS has none of the file columns.
wxExListItemWithFileName::wxExListItemWithFileName(
  wxExListView* lv, 
  const int itemnumber)
  : wxExListItem(lv, itemnumber)
  , m_FileName(
      (!GetColumnText(_("File Name"), false).empty() ?
          GetColumnText(_("In Folder"), false) + wxFileName::GetPathSeparator() +
          GetColumnText(_("File Name"), false) : wxString(wxEmptyString))
      )
  , m_FileSpec(GetColumnText(_("Type"), false))
{
  m_IsReadOnly = (GetListView()->GetItemData(itemnumber) > 0);
}

wxExListItemWithFileName::wxExListItemWithFileName(
  wxExListView* listview,
  const wxString& fullpath,
  const wxString& filespec)
  : wxExListItem(listview, -1)
  , m_FileName(fullpath)
  , m_FileSpec(filespec)
{
}

void wxExListItemWithFileName::Insert(long index)
{
  SetId(index == -1 ? GetListView()->GetItemCount(): index);
  const int col = GetListView()->FindColumn(_("File Name"), false);
  const wxString filename = (
    m_FileName.FileExists() || m_FileName.DirExists() ?
      m_FileName.GetFullName():
      m_FileName.GetFullPath());

  if (col == 0)
  {
    SetColumn(col); // do not combine this with next statement in SetColumnText!!
    SetText(filename);
  }

  GetListView()->InsertItem(*this);
  
  if (GetListView()->IsShown())
  {
#if wxUSE_STATUSBAR
    GetListView()->UpdateStatusBar();
#endif
  }

  SetImage(m_FileName.GetIconID());

  Update();

  if (col > 0)
  {
    SetColumnText(col, filename);
  }
}

const wxExFileStatistics& wxExListItemWithFileName::Run(const wxExTool& tool)
{
#if wxUSE_STATUSBAR
  wxExFrame::StatusText(m_FileName.GetFullPath());
#endif

  if (m_FileName.FileExists())
  {
    wxExTextFileWithListView file(m_FileName, tool);

    if (file.RunTool())
    {
      if (tool.IsRCSType())
      {
        Update();
      }

      m_Statistics += file.GetStatistics();
    }
  }
  else
  {
    wxExDirTool dir(tool, m_FileName.GetFullPath(), m_FileSpec);

    if (dir.FindFiles())
    {
      m_Statistics += dir.GetStatistics();

      // Here we show the counts of individual folders on the top level.
      if (tool.IsCount() && GetListView()->GetSelectedItemCount() > 1)
      {
        tool.Log(&m_Statistics.GetElements(), m_FileName.GetFullPath());
      }
    }
  }

  return m_Statistics;
}

void wxExListItemWithFileName::SetReadOnly(bool readonly)
{
  if (readonly)
  {
    SetTextColour(wxColour("RED"));
  }
  else
  {
    SetTextColour(*wxBLACK);
  }

  // Using GetTextColour did not work, so keep state in boolean.
  m_IsReadOnly = readonly;
  GetListView()->SetItemData(GetId(), m_IsReadOnly);
}

void wxExListItemWithFileName::Update()
{
  SetReadOnly(m_FileName.GetStat().IsReadOnly());

  if (!GetListView()->SetItem(*this))
  {
    wxFAIL;
    return;
  }

  if (m_FileName.FileExists() ||
      wxFileName::DirExists(m_FileName.GetFullPath()))
  {
    const unsigned long size = m_FileName.GetStat().st_size; // to prevent warning
    SetColumnText(_("Type"),
      (wxFileName::DirExists(m_FileName.GetFullPath()) ?
         m_FileSpec:
         m_FileName.GetExt()));
    SetColumnText(_("In Folder"), m_FileName.GetPath());
    SetColumnText(_("Size"),
      (!wxFileName::DirExists(m_FileName.GetFullPath()) ?
         (wxString::Format("%lu", size)):
          wxString(wxEmptyString)));
    SetColumnText(_("Modified"), m_FileName.GetStat().GetModificationTime());
  }
}

void wxExListItemWithFileName::UpdateRevisionList(const wxExRCS& rcs)
{
  SetColumnText(_("Revision"), rcs.GetRevisionNumber());
  SetColumnText(_("Date"), rcs.GetRevisionTime().FormatISODate());
  SetColumnText(_("Initials"), rcs.GetUser());
  SetColumnText(_("Revision Comment"), rcs.GetDescription());
}
