/******************************************************************************\
* File:          listitem.cpp
* Purpose:       Implementation of class 'ftListItem'
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2008 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/filetool/filetool.h>

// Do not give an error if columns do not exist.
// E.g. the LIST_PROCESS has none of the file columns.
ftListItem::ftListItem(exListView* lv, const int itemnumber)
  : exListItem(lv, itemnumber)
  , m_Statistics(
      GetColumnText(_("In Folder"), false),
      GetColumnText(_("File Name"), false))
  , m_FileSpec(GetColumnText(_("Type"), false))
{
}

ftListItem::ftListItem(
  exListView* listview,
  const wxString& fullpath,
  const wxString& filespec)
  : exListItem(listview, -1)
  , m_Statistics(fullpath)
  , m_FileSpec(filespec)
{
}

void ftListItem::Insert(long index)
{
  SetId(index == -1 ? GetListView()->GetItemCount(): index);
  const long col = GetListView()->FindColumn(_("File Name"), false);
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
  GetListView()->UpdateStatusBar();

  SetImage(ftGetFileIcon(&m_Statistics));

  Update();

  if (col > 0)
  {
    SetColumnText(col, filename);
  }
}

bool ftListItem::Run(const exTool& tool, ftListView* listview)
{
  exFrame::StatusText(m_Statistics.GetFullPath());

  if (m_Statistics.FileExists())
  {
    ftTextFile file(m_Statistics);

    if (file.RunTool())
    {
      if (tool.IsRCSType())
      {
        Update();
      }

      m_Statistics += file.GetStatistics();

      return true;
    }
    else
    {
      return false;
    }
  }
  else
  {
    ftDir dir(listview, m_Statistics.GetFullPath(), m_FileSpec);

    if (dir.RunTool())
    {
      m_Statistics += dir.GetStatistics();

      // Here we show the counts of individual folders on the top level.
      if (tool.IsCountType() && GetListView()->GetSelectedItemCount() > 1)
      {
        m_Statistics.Log();
      }

      return true;
    }
    else
    {
      return false;
    }
  }
}

void ftListItem::Update()
{
  SetBackgroundColour(m_Statistics.GetStat().GetColour());

  // Update readonly state in listview item data.
  // SetData does not work, as list items are constructed/destructed a lot.
  GetListView()->SetItemData(GetId(), m_Statistics.GetStat().IsReadOnly());

  const int fontstyle = (m_Statistics.GetStat().IsReadOnly() ? wxFONTSTYLE_ITALIC: wxFONTSTYLE_NORMAL);

  wxFont font(
    exApp::GetConfig(_("List Font") + "/Size", 10),
    wxFONTFAMILY_DEFAULT,
    fontstyle,
    wxFONTWEIGHT_NORMAL,
    false,
    exApp::GetConfig(_("List Font") + "/Name", "courier new"));

  SetFont(font);
  GetListView()->SetItem(*this);

  if (m_Statistics.GetStat().IsLink())
  {
    SetTextColour(m_Statistics.GetStat().GetLinkColour());
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

void ftListItem::UpdateRevisionList(
  const exTextFile* file,
  const wxString& format)
{
  SetColumnText(_("Revision"), file->GetRevisionNumber());
  SetColumnText(_("Date"), file->GetRevisionTime().Format(format));
  SetColumnText(_("Initials"), file->GetUser());
  SetColumnText(_("Revision Comment"), file->GetDescription());
}
