////////////////////////////////////////////////////////////////////////////////
// Name:      listitem.cpp
// Purpose:   Implementation of class 'wex::listitem'
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/config.h>
#include <wex/frame.h>
#include <wex/lexers.h>
#include <wex/listitem.h>
#include <wex/log.h>
#include <wex/util.h>

// Do not give an error if columns do not exist.
// E.g. the LIST_PROCESS has none of the file columns.
wex::listitem::listitem(
  listview* lv, 
  long itemnumber)
  : m_ListView(lv)
  , m_Path(
    (!lv->get_item_text(itemnumber, _("File Name")).empty() &&
     !lv->get_item_text(itemnumber, _("In Folder")).empty() ?
        path(
          lv->get_item_text(itemnumber, _("In Folder")),
          lv->get_item_text(itemnumber, _("File Name"))) : 
        path(lv->get_item_text(itemnumber))))
  , m_FileSpec(lv->get_item_text(itemnumber, _("Type")))
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

void wex::listitem::insert(long index)
{
  SetId(index == -1 ? m_ListView->GetItemCount(): index);
  
  int col = 0;
  std::string filename;
  
  if (m_ListView->InReportView())
  {
    col = m_ListView->find_column(_("File Name"));
    assert(col >= 0);
    filename = (m_Path.file_exists() || m_Path.dir_exists() ? 
      m_Path.fullname(): m_Path.string());
  }
  else
  {
    filename = m_Path.string();
  }

  if (col == 0)
  {
    SetColumn(col); // do not combine this with next statement in SetItem!!
    SetText(filename);
  }

  ((wxListView* )m_ListView)->InsertItem(*this);
  
  frame::update_statusbar(m_ListView);

  update();

  if (col > 0)
  {
    m_ListView->SetItem(GetId(), col, filename);
  }
}

std::stringstream wex::listitem::log() const
{
  std::stringstream ss;

  ss << "PATH: " << m_Path.string();

  return ss;
}

bool wex::listitem::set_item(
  const std::string& col_name, const std::string& text) 
{
  if (const auto col = m_ListView->find_column(col_name); col != -1)
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
  if (!readonly)
  {
    lexers::get()->apply_default_style(nullptr,
      [=](const std::string& fore) {
        SetTextColour(wxColour(fore));});
  }
  else
  {
    SetTextColour(config(_("Readonly colour")).get(*wxRED));
  }

  ((wxListView* )m_ListView)->SetItem(*this);

  // Using GetTextColour did not work, so keep state in boolean.
  m_IsReadOnly = readonly;
  m_ListView->SetItemData(GetId(), m_IsReadOnly);
}

void wex::listitem::update()
{
  SetImage(
    m_ListView->data().image() == listview_data::IMAGE_FILE_ICON && 
    m_Path.stat().is_ok() ? get_iconid(m_Path): -1);

  SetReadOnly(m_Path.stat().is_readonly());

  ((wxListView *)m_ListView)->SetItem(*this);

  if (
     m_ListView->InReportView() &&
     m_Path.stat().is_ok())
  {
    set_item(_("Type"),
      m_Path.dir_exists() ? m_FileSpec: m_Path.extension());
    set_item(_("In Folder"), m_Path.get_path());
    set_item(_("Modified"), m_Path.stat().get_modification_time());
  
    if (m_Path.file_exists())
    {
      set_item(_("Size"), std::to_string(m_Path.stat().st_size));
    }
  }
}
