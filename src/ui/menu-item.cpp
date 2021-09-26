////////////////////////////////////////////////////////////////////////////////
// Name:      menu.cpp
// Purpose:   Implementation of wex::menu_item class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/factory/defs.h>
#include <wex/ui/art.h>
#include <wex/ui/frame.h>
#include <wex/ui/menu.h>

wex::menu_item::menu_item(type_t type)
  : m_type(type)
{
}

wex::menu_item::menu_item(
  wxWindowID         id,
  const std::string& name,
  const data::menu&  data)
  : m_id(id)
  , m_type(MENU)
  , m_data(data)
  , m_name(name)
{
  m_data.bind(m_id);
}

wex::menu_item::menu_item(
  wxWindowID         id,
  const std::string& name,
  type_t             type,
  const data::menu&  data)
  : m_id(id)
  , m_type(type)
  , m_name(name)
  , m_data(data)
{
  m_data.bind(m_id);
}

wex::menu_item::menu_item(
  wex::menu*         submenu,
  const std::string& name,
  wxWindowID         id,
  const data::menu&  data)
  : m_id(id)
  , m_type(SUBMENU)
  , m_name(name)
  , m_menu(submenu)
  , m_data(data)
{
  m_data.bind(m_id);
}

wex::menu_item::menu_item(
  const wex::path&  p,
  frame*            frame,
  bool              show_modal,
  const data::menu& data)
  : m_type(VCS)
  , m_frame(frame)
  , m_path(p)
  , m_modal(show_modal)
{
  m_data.bind(m_id);
}

wex::menu_item::menu_item(
  wxWindowID        id,
  file_history&     history,
  const data::menu& data)
  : m_id(id)
  , m_type(HISTORY)
  , m_history(&history)
  , m_data(data)
{
  m_data.bind(m_id);
}

wex::menu_item::menu_item(const frame* frame)
  : m_frame(frame)
  , m_type(PANES)
{
}

void wex::menu_item::append(wex::menu* menu) const
{
  switch (m_type)
  {
    case CHECK:
      menu->AppendCheckItem(m_id, m_name, m_data.help_text());
      break;

    case HISTORY:
      m_history->use_menu(m_id, menu);
      break;

    case MENU:
    {
      auto* item = new wxMenuItem(menu, m_id, m_name, m_data.help_text());

      if (const stockart art(m_id); art.get_bitmap().IsOk())
      {
        item->SetBitmap(art.get_bitmap(
          wxART_MENU,
          wxArtProvider::GetSizeHint(wxART_MENU, true)));
      }
      else if (!m_data.art().empty())
      {
        if (const wxBitmap bitmap(wxArtProvider::GetBitmap(
              m_data.art(),
              wxART_MENU,
              wxArtProvider::GetSizeHint(wxART_MENU, true)));
            bitmap.IsOk())
        {
          item->SetBitmap(bitmap);
        }
      }

      menu->Append(item);
    }
    break;

    case PANES:
      append_panes(menu);
      break;

    case RADIO:
      menu->AppendRadioItem(m_id, m_name, m_data.help_text());
      break;

    case SUBMENU:
      if (m_id == wxID_ANY)
      {
        menu->AppendSubMenu(m_menu, m_name, m_data.help_text());
      }
      else
      {
        // This one is deprecated, but is necessary if
        // we have an explicit itemid.
        menu->Append(m_id, m_name, m_menu, m_data.help_text());
      }
      break;

    case VCS:
      append_vcs(menu);
      break;

    default:
      assert(0);
  }
}

void wex::menu_item::append_panes(wex::menu* menu) const
{
  menu->append({
#ifdef __WXMSW__
    {ID_VIEW_MENUBAR, _("&Menubar\tCtrl+M"), CHECK},
#endif
    {ID_VIEW_STATUSBAR, _("&Statusbar"), CHECK}});

  for (const auto& it : m_frame->toggled_panes())
  {
    menu->append({{it.second, it.first.second, CHECK}});
  }
}

void wex::menu_item::append_vcs(wex::menu* menu) const
{
  m_frame->append_vcs(menu, this);
}
