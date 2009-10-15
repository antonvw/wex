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

#include <wx/extension/report/listitem.h>
#include <wx/extension/app.h>
#include <wx/extension/frame.h>
#include <wx/extension/report/textfile.h>
#include <wx/extension/report/util.h>

// Do not give an error if columns do not exist.
// E.g. the LIST_PROCESS has none of the file columns.
wxExListItemWithFileName::wxExListItemWithFileName(
  wxExListView* lv, 
  const int itemnumber)
  : wxExListItem(lv, itemnumber)
  , m_Statistics(
      (!GetColumnText(_("File Name"), false).empty() ?
          GetColumnText(_("In Folder"), false) + wxFileName::GetPathSeparator() +
          GetColumnText(_("File Name"), false) : wxString(wxEmptyString))
      )
  , m_FileSpec(GetColumnText(_("Type"), false))
{
}

wxExListItemWithFileName::wxExListItemWithFileName(
  wxExListView* listview,
  const wxString& fullpath,
  const wxString& filespec)
  : wxExListItem(listview, -1)
  , m_Statistics(fullpath)
  , m_FileSpec(filespec)
{
}

void wxExListItemWithFileName::Insert(long index)
{
  SetId(index == -1 ? GetListView()->GetItemCount(): index);
  const int col = GetListView()->FindColumn(_("File Name"), false);
  const wxString filename = (
    m_Statistics.FileExists() || m_Statistics.DirExists() ?
      m_Statistics.GetFullName():
      m_Statistics.GetFullPath());

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

  SetImage(m_Statistics.GetIconID());

  Update();

  if (col > 0)
  {
    SetColumnText(col, filename);
  }
}

const wxExFileNameStatistics wxExListItemWithFileName::Run(const wxExTool& tool)
{
#if wxUSE_STATUSBAR
  wxExFrame::StatusText(m_Statistics.GetFullPath());
#endif

  if (m_Statistics.FileExists())
  {
    wxExTextFileWithListView file(m_Statistics, tool);

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
    wxExDirWithListView dir(tool, m_Statistics.GetFullPath(), m_FileSpec);

    if (dir.FindFiles())
    {
      m_Statistics += dir.GetStatistics();

      // Here we show the counts of individual folders on the top level.
      if (tool.IsCount() && GetListView()->GetSelectedItemCount() > 1)
      {
        m_Statistics.Log(tool);
      }
    }
  }

  return m_Statistics;
}

void wxExListItemWithFileName::Update()
{
  // Update readonly state in listview item data.
  // SetData does not work, as list items are constructed/destructed a lot.
  GetListView()->SetItemData(GetId(), m_Statistics.GetStat().IsReadOnly());

  const int fontstyle = (m_Statistics.GetStat().IsReadOnly() ? 
    wxFONTSTYLE_ITALIC: 
    wxFONTSTYLE_NORMAL);

  wxFont font(
    wxExApp::GetConfig(_("List Font") + "/Size", 10),
    wxFONTFAMILY_DEFAULT,
    fontstyle,
    wxFONTWEIGHT_NORMAL,
    false,
    wxExApp::GetConfig(_("List Font") + "/Name", "courier new"));

  SetFont(font);

  if (!GetListView()->SetItem(*this))
  {
    wxFAIL;
    return;
  }

  if (m_Statistics.FileExists() ||
      wxFileName::DirExists(m_Statistics.GetFullPath()))
  {
    const unsigned long size = m_Statistics.GetStat().st_size; // to prevent warning
    SetColumnText(_("Type"),
      (wxFileName::DirExists(m_Statistics.GetFullPath()) ?
         m_FileSpec:
         m_Statistics.GetExt()));
    SetColumnText(_("In Folder"), m_Statistics.GetPath());
    SetColumnText(_("Size"),
      (!wxFileName::DirExists(m_Statistics.GetFullPath()) ?
         (wxString::Format("%lu", size)):
          wxString(wxEmptyString)));
    SetColumnText(_("Modified"), m_Statistics.GetStat().GetModificationTime());
  }
}

void wxExListItemWithFileName::UpdateRevisionList(const wxExRCS& rcs)
{
  SetColumnText(_("Revision"), rcs.GetRevisionNumber());
  SetColumnText(_("Date"), rcs.GetRevisionTime().FormatISOCombined(' '));
  SetColumnText(_("Initials"), rcs.GetUser());
  SetColumnText(_("Revision Comment"), rcs.GetDescription());
}
