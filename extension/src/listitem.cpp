////////////////////////////////////////////////////////////////////////////////
// Name:      listitem.cpp
// Purpose:   Implementation of class 'wxExListItem'
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/extension/frame.h>
#include <wx/extension/listitem.h>
#include <wx/extension/util.h>

// Do not give an error if columns do not exist.
// E.g. the LIST_PROCESS has none of the file columns.
wxExListItem::wxExListItem(
  wxExListView* lv, 
  long itemnumber)
  : m_ListView(lv)
  , m_FileName(
    (!lv->GetItemText(itemnumber, _("File Name")).empty() ?
        wxFileName(
          lv->GetItemText(itemnumber, _("In Folder")),
          lv->GetItemText(itemnumber, _("File Name"))) : 
        lv->GetItemText(itemnumber)))
  , m_FileSpec(lv->GetItemText(itemnumber, _("Type")))
{
  SetId(itemnumber);
  m_IsReadOnly = (m_ListView->GetItemData(GetId()) > 0);
}

wxExListItem::wxExListItem(
  wxExListView* listview,
  const wxExFileName& filename,
  const wxString& filespec)
  : m_ListView(listview)
  , m_FileName(filename)
  , m_FileSpec(filespec)
{
  SetId(-1);
}

void wxExListItem::Insert(long index)
{
  SetId(index == -1 ? m_ListView->GetItemCount(): index);
  
  int col = 0;
  wxString filename;
  
  if (m_ListView->GetWindowStyle() & wxLC_REPORT)
  {
    col = m_ListView->FindColumn(_("File Name"));
    filename = (
      m_FileName.FileExists() || m_FileName.DirExists() ?
        m_FileName.GetFullName():
        m_FileName.GetFullPath());
  }
  else
  {
    col = 0;
    filename = m_FileName.GetFullPath();
  }

  if (col == 0)
  {
    SetColumn(col); // do not combine this with next statement in SetItem!!
    SetText(filename);
  }

  m_ListView->InsertItem(*this);
  
#if wxUSE_STATUSBAR
  wxExFrame::UpdateStatusBar(m_ListView);
#endif

  if (m_FileName.GetStat().IsOk())
  {
    SetImage(wxExGetIconID(m_FileName));
  }
  else
  {
    SetImage(-1);
  }

  if (!m_ListView->SetItem(*this))
  {
    wxFAIL;
    return;
  }

  Update();

  if (col > 0)
  {
    SetItem(col, filename);
  }
}

void wxExListItem::SetItem(int col_number, const wxString& text) 
{
  if (col_number != -1)
  {
    m_ListView->SetItem(GetId(), col_number, text);
  }
}

void wxExListItem::SetReadOnly(bool readonly)
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

  m_ListView->SetItem(*this);

  // Using GetTextColour did not work, so keep state in boolean.
  m_IsReadOnly = readonly;
  m_ListView->SetItemData(GetId(), m_IsReadOnly);
}

void wxExListItem::Update()
{
  SetReadOnly(m_FileName.GetStat().IsReadOnly());

  if (
    (m_ListView->GetWindowStyle() & wxLC_REPORT) &&
     m_FileName.GetStat().IsOk())
  {
    const unsigned long size = m_FileName.GetStat().st_size; // to prevent warning
    
    SetItem(_("Type"),
      (wxFileName::DirExists(m_FileName.GetFullPath()) ? // IsDir not ok
         m_FileSpec:
         m_FileName.GetExt()));
    SetItem(_("In Folder"), m_FileName.GetPath());
    SetItem(_("Size"),
      (!wxFileName::DirExists(m_FileName.GetFullPath()) ? // IsDir not ok
         (wxString::Format("%lu", size)):
          wxString(wxEmptyString)));
    SetItem(_("Modified"), m_FileName.GetStat().GetModificationTime());
  }
}

void wxExListItem::UpdateRevisionList(const wxExRCS& rcs)
{
  SetItem(_("Revision"), rcs.GetRevisionNumber());
  SetItem(_("Date"), rcs.GetRevisionTime().FormatISOCombined(' '));
  SetItem(_("Initials"), rcs.GetUser());
  SetItem(_("Revision Comment"), rcs.GetDescription());
}
