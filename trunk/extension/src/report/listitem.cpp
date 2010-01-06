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
#include <wx/extension/frame.h>
#include <wx/extension/report/listitem.h>
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
      (!lv->GetItemText(itemnumber, _("File Name"), false).empty() ?
          lv->GetItemText(itemnumber, _("In Folder"), false) + wxFileName::GetPathSeparator() +
          lv->GetItemText(itemnumber, _("File Name"), false) : wxString(wxEmptyString))
      )
  , m_FileSpec(lv->GetItemText(itemnumber, _("Type"), false))
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
    SetColumn(col); // do not combine this with next statement in SetItemText!!
    SetText(filename);
  }

  GetListView()->InsertItem(*this);
  
  if (GetListView()->IsShown())
  {
#if wxUSE_STATUSBAR
    GetListView()->UpdateStatusBar();
#endif
  }

  if (m_FileName.GetStat().IsOk())
  {
    SetImage(m_FileName.GetIconID());
  }

  Update();

  if (col > 0)
  {
    SetItemText(col, filename);
  }
}

const wxExFileStatistics wxExListItemWithFileName::Run(const wxExTool& tool)
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
    }

    return file.GetStatistics();
  }
  else
  {
    wxExDirTool dir(tool, m_FileName.GetFullPath(), m_FileSpec);

    if (dir.FindFiles())
    {
      // Here we show the counts of individual folders on the top level.
      if (tool.IsCount() && GetListView()->GetSelectedItemCount() > 1)
      {
        tool.Log(&dir.GetStatistics().GetElements(), m_FileName.GetFullPath());
      }
    }

    return dir.GetStatistics();
  }
}

void wxExListItemWithFileName::SetReadOnly(bool readonly)
{
  if (readonly)
  {
    SetTextColour(wxConfigBase::Get()->ReadObject(
      _("List Colour"), wxColour("RED")));
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
    SetItemText(_("Type"),
      (wxFileName::DirExists(m_FileName.GetFullPath()) ?
         m_FileSpec:
         m_FileName.GetExt()));
    SetItemText(_("In Folder"), m_FileName.GetPath());
    SetItemText(_("Size"),
      (!wxFileName::DirExists(m_FileName.GetFullPath()) ?
         (wxString::Format("%lu", size)):
          wxString(wxEmptyString)));
    SetItemText(_("Modified"), m_FileName.GetStat().GetModificationTime());
  }
}

void wxExListItemWithFileName::UpdateRevisionList(const wxExRCS& rcs)
{
  SetItemText(_("Revision"), rcs.GetRevisionNumber());
  SetItemText(_("Date"), rcs.GetRevisionTime().FormatISOCombined(' '));
  SetItemText(_("Initials"), rcs.GetUser());
  SetItemText(_("Revision Comment"), rcs.GetDescription());
}
