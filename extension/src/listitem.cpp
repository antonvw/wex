////////////////////////////////////////////////////////////////////////////////
// Name:      listitem.cpp
// Purpose:   Implementation of class 'wxExListItem'
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/filename.h>
#include <wx/extension/frame.h>
#include <wx/extension/listitem.h>
#include <wx/extension/util.h>

// Do not give an error if columns do not exist.
// E.g. the LIST_PROCESS has none of the file columns.
wxExListItem::wxExListItem(
  wxExListView* lv, 
  long itemnumber)
  : m_ListView(lv)
  , m_Path(
    (!lv->GetItemText(itemnumber, _("File Name").ToStdString()).empty() &&
     !lv->GetItemText(itemnumber, _("In Folder").ToStdString()).empty() ?
        wxExPath(
          lv->GetItemText(itemnumber, _("In Folder").ToStdString()),
          lv->GetItemText(itemnumber, _("File Name").ToStdString())) : 
        wxExPath(lv->GetItemText(itemnumber))))
  , m_FileSpec(lv->GetItemText(itemnumber, _("Type").ToStdString()))
{
  SetId(itemnumber);
  m_IsReadOnly = (m_ListView->GetItemData(GetId()) > 0);
}

wxExListItem::wxExListItem(
  wxExListView* listview,
  const wxExPath& filename,
  const std::string& filespec)
  : m_ListView(listview)
  , m_Path(filename)
  , m_FileSpec(filespec)
  , m_IsReadOnly(false)
{
  SetId(-1);
}

void wxExListItem::Insert(long index)
{
  SetId(index == -1 ? m_ListView->GetItemCount(): index);
  
  int col = 0;
  std::string filename;
  
  if (m_ListView->InReportView())
  {
    col = m_ListView->FindColumn(_("File Name").ToStdString());
    wxASSERT(col >= 0);
    filename = (
      m_Path.FileExists() || m_Path.DirExists() ? m_Path.GetFullName(): m_Path.Path().string());
  }
  else
  {
    filename = m_Path.Path().string();
  }

  if (col == 0)
  {
    SetColumn(col); // do not combine this with next statement in SetItem!!
    SetText(filename);
  }

  ((wxListView* )m_ListView)->InsertItem(*this);
  
#if wxUSE_STATUSBAR
  wxExFrame::UpdateStatusBar(m_ListView);
#endif

  Update();

  if (col > 0)
  {
    m_ListView->SetItem(GetId(), col, filename);
  }
}

void wxExListItem::SetItem(const std::string& col_name, const std::string& text) 
{
  const auto col = m_ListView->FindColumn(col_name);
  
  if (col != -1)
  {
    m_ListView->SetItem(GetId(), col, text);
  }
}

void wxExListItem::SetReadOnly(bool readonly)
{
  SetTextColour(readonly ? 
    wxConfigBase::Get()->ReadObject(_("Readonly colour"), *wxRED):
    wxConfigBase::Get()->ReadObject(_("Foreground colour"), *wxBLACK));

  ((wxListView* )m_ListView)->SetItem(*this);

  // Using GetTextColour did not work, so keep state in boolean.
  m_IsReadOnly = readonly;
  m_ListView->SetItemData(GetId(), m_IsReadOnly);
}

void wxExListItem::Update()
{
  SetImage(
    m_ListView->GetData().Image() == IMAGE_FILE_ICON && 
    m_Path.GetStat().IsOk() ? wxExGetIconID(m_Path): -1);

  ((wxListView *)m_ListView)->SetItem(*this);

  SetReadOnly(m_Path.GetStat().IsReadOnly());

  if (
     m_ListView->InReportView() &&
     m_Path.GetStat().IsOk())
  {
    SetItem(_("Type").ToStdString(),
      m_Path.DirExists() ? m_FileSpec: m_Path.GetExtension().substr(1));
    SetItem(_("In Folder").ToStdString(), m_Path.GetPath());
    SetItem(_("Size").ToStdString(),
      m_Path.FileExists() ? std::to_string(m_Path.GetStat().st_size): std::string());
    SetItem(_("Modified").ToStdString(), m_Path.GetStat().GetModificationTime());
  }
}
