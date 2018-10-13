////////////////////////////////////////////////////////////////////////////////
// Name:      listitem.cpp
// Purpose:   Implementation of class 'wex::listitem'
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/extension/frame.h>
#include <wx/extension/listitem.h>
#include <wx/extension/log.h>
#include <wx/extension/util.h>

// Do not give an error if columns do not exist.
// E.g. the LIST_PROCESS has none of the file columns.
wex::listitem::listitem(
  listview* lv, 
  long itemnumber)
  : m_ListView(lv)
  , m_Path(
    (!lv->GetItemText(itemnumber, _("File Name")).empty() &&
     !lv->GetItemText(itemnumber, _("In Folder")).empty() ?
        path(
          lv->GetItemText(itemnumber, _("In Folder")),
          lv->GetItemText(itemnumber, _("File Name"))) : 
        path(lv->GetItemText(itemnumber))))
  , m_FileSpec(lv->GetItemText(itemnumber, _("Type")))
{
  SetId(itemnumber);
  m_IsReadOnly = (m_ListView->GetItemData(GetId()) > 0);
}

wex::listitem::listitem(
  listview* listview,
  const path& filename,
  const std::string& filespec)
  : m_ListView(listview)
  , m_Path(filename)
  , m_FileSpec(filespec)
  , m_IsReadOnly(false)
{
  SetId(-1);
}

void wex::listitem::Insert(long index)
{
  SetId(index == -1 ? m_ListView->GetItemCount(): index);
  
  int col = 0;
  std::string filename;
  
  if (m_ListView->InReportView())
  {
    col = m_ListView->FindColumn(_("File Name"));
    wxASSERT(col >= 0);
    filename = (m_Path.FileExists() || m_Path.DirExists() ? 
      m_Path.GetFullName(): m_Path.Path().string());
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
  
  frame::UpdateStatusBar(m_ListView);

  Update();

  if (col > 0)
  {
    m_ListView->SetItem(GetId(), col, filename);
  }
}

bool wex::listitem::SetItem(
  const std::string& col_name, const std::string& text) 
{
  if (text.empty())
  {
    log() << *this << col_name << "empty";
    return false;
  }
  
  if (const auto col = m_ListView->FindColumn(col_name); col != -1)
  {
    if (!m_ListView->SetItem(GetId(), col, text))
    {
      log() << *this << "col:" << col << "id:" << GetId() << "text:" << text;
      return false;
    }
  }
  else
  {
    log() << *this << col_name << "unknown";
    return false;
  }
  
  return true;
}

void wex::listitem::SetReadOnly(bool readonly)
{
  SetTextColour(readonly ? 
    wxConfigBase::Get()->ReadObject(_("Readonly colour"), *wxRED):
    wxConfigBase::Get()->ReadObject(_("Foreground colour"), *wxBLACK));

  ((wxListView* )m_ListView)->SetItem(*this);

  // Using GetTextColour did not work, so keep state in boolean.
  m_IsReadOnly = readonly;
  m_ListView->SetItemData(GetId(), m_IsReadOnly);
}

void wex::listitem::Update()
{
  SetImage(
    m_ListView->GetData().Image() == IMAGE_FILE_ICON && 
    m_Path.GetStat().IsOk() ? get_iconid(m_Path): -1);

  ((wxListView *)m_ListView)->SetItem(*this);

  SetReadOnly(m_Path.GetStat().IsReadOnly());

  if (
     m_ListView->InReportView() &&
     m_Path.GetStat().IsOk())
  {
    SetItem(_("Type"),
      m_Path.DirExists() ? m_FileSpec: m_Path.GetExtension());
    SetItem(_("In Folder"), m_Path.GetPath());
    SetItem(_("Modified"), m_Path.GetStat().GetModificationTime());
  
    if (m_Path.FileExists())
    {
      SetItem(_("Size"), std::to_string(m_Path.GetStat().st_size));
    }
  }
}
