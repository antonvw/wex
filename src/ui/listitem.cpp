////////////////////////////////////////////////////////////////////////////////
// Name:      listitem.cpp
// Purpose:   Implementation of class wex::listitem
// Author:    Anton van Wezenbeek
// Copyright: (c) 2009-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/config.h>
#include <wex/core/log.h>
#include <wex/syntax/lexers.h>
#include <wex/ui/listitem.h>

#include <wx/generic/dirctrlg.h>

// Do not give an error if columns do not exist.
// E.g. the LIST_PROCESS has none of the file columns.
wex::listitem::listitem(listview* lv, long itemnumber)
  : listitem(
      lv,
      !lv->get_item_text(itemnumber, _("File Name")).empty() &&
          !lv->get_item_text(itemnumber, _("In Folder")).empty() ?
        wex::path(
          wex::path(lv->get_item_text(itemnumber, _("In Folder"))),
          lv->get_item_text(itemnumber, _("File Name")),
          path::log_t().set(path::LOG_MOD)) :
        wex::path(
          lv->get_item_text(itemnumber),
          path::log_t().set(path::LOG_MOD)),
      lv->get_item_text(itemnumber, _("Type")))
{
  SetId(itemnumber);

  if (itemnumber >= 0)
  {
    m_is_readonly = (m_listview->GetItemData(GetId()) > 0);
  }
}

wex::listitem::listitem(
  listview*          listview,
  const wex::path&   filename,
  const std::string& filespec)
  : m_listview(listview)
  , m_path(filename)
  , m_file_spec(filespec)
{
  SetId(-1);
}

void wex::listitem::insert(long index)
{
  SetId(index == -1 ? m_listview->GetItemCount() : index);

  int         col = 0;
  std::string filename;

  if (m_listview->InReportView())
  {
    col = m_listview->find_column(_("File Name"));
    assert(col >= 0);
    filename = (m_path.exists() ? m_path.filename() : m_path.string());
  }
  else
  {
    filename = m_path.string();
  }

  if (col == 0)
  {
    SetColumn(col); // do not combine this with next statement in SetItem!!
    SetText(filename);
  }

  m_listview->InsertItem(*this);

  update();

  if (col > 0)
  {
    m_listview->SetItem(GetId(), col, filename);
  }
}

std::stringstream wex::listitem::log() const
{
  std::stringstream ss;
  using boost::describe::operators::operator<<;
  ss << "listitem: " << *this << GetId();
  return ss;
}

bool wex::listitem::set_item(
  const std::string& col_name,
  const std::string& text)
{
  if (const auto col = m_listview->find_column(col_name); col != -1)
  {
    if (!m_listview->SetItem(GetId(), col, text))
    {
      log() << *this << "col:" << col << "text:" << text;
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

void wex::listitem::set_readonly(bool readonly)
{
  if (!readonly)
  {
    lexers::get()->apply_default_style(
      nullptr,
      [=, this](const std::string& fore)
      {
        SetTextColour(wxColour(fore));
      });
  }
  else
  {
    SetTextColour(config(_("list.Readonly colour")).get(*wxLIGHT_GREY));
  }

  m_listview->SetItem(*this);

  // Using GetTextColour did not work, so keep state in boolean.
  m_is_readonly = readonly;
  m_listview->SetItemData(GetId(), m_is_readonly);
}

void wex::listitem::update()
{
  if (m_path.dir_exists())
  {
    SetImage(wxFileIconsTable::folder);
  }
  else if (
    m_listview->data().image() == data::listview::IMAGE_FILE_ICON &&
    m_path.stat().is_ok())
  {
    SetImage(wxFileIconsTable::file);
  }

  set_readonly(m_path.stat().is_readonly());

  m_listview->SetItem(*this);

  if (m_listview->InReportView() && m_path.stat().is_ok())
  {
    set_item(_("Type"), m_path.dir_exists() ? m_file_spec : m_path.extension());
    set_item(_("In Folder"), m_path.parent_path());
    set_item(_("Modified"), m_path.stat().get_modification_time_str());

    if (m_path.file_exists())
    {
      set_item(_("Size"), std::to_string(m_path.stat().get_size()));
    }
  }
}
