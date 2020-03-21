////////////////////////////////////////////////////////////////////////////////
// Name:      menu.cpp
// Purpose:   Implementation of wex::menu_item class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/art.h>
#include <wex/defs.h>
#include <wex/managedframe.h>
#include <wex/menu-item.h>
#include <wex/menu.h>
#include <wex/process.h>
#include <wex/vcs.h>
#include <wx/menu.h>

wex::menu_item::menu_item(type_t type)
  : m_type(type)
{
}

wex::menu_item::menu_item(
  int                                   id,
  const std::string&                    name,
  const std::string&                    help,
  const wxArtID&                        art,
  std::function<void(wxCommandEvent&)>  action,
  std::function<void(wxUpdateUIEvent&)> ui)
  : m_id(id)
  , m_type(MENU)
  , m_name(name)
  , m_help_text(help)
  , m_artid(art)
{
  auto* frame = dynamic_cast<managed_frame*>(wxTheApp->GetTopWindow());

  if (action != nullptr)
  {
    frame->Bind(wxEVT_MENU, action, m_id);
  }

  if (ui != nullptr)
  {
    frame->Bind(wxEVT_UPDATE_UI, ui, m_id);
  }
}

wex::menu_item::menu_item(
  int                                   id,
  const std::string&                    name,
  type_t                                type,
  std::function<void(wxCommandEvent&)>  action,
  std::function<void(wxUpdateUIEvent&)> ui,
  const std::string&                    help)
  : m_id(id)
  , m_type(type)
  , m_help_text(help)
  , m_name(name)
{
  auto* frame = dynamic_cast<managed_frame*>(wxTheApp->GetTopWindow());

  if (action != nullptr)
  {
    frame->Bind(wxEVT_MENU, action, m_id);
  }

  if (ui != nullptr)
  {
    frame->Bind(wxEVT_UPDATE_UI, ui, m_id);
  }
}

wex::menu_item::menu_item(
  wex::menu*         submenu,
  const std::string& name,
  const std::string& help,
  int                id)
  : m_id(id)
  , m_type(SUBMENU)
  , m_name(name)
  , m_help_text(help)
  , m_menu(submenu)
{
}

wex::menu_item::menu_item(const wex::path& p, bool show_modal)
  : m_type(VCS)
  , m_path(p)
  , m_modal(show_modal)
{
}

wex::menu_item::menu_item(
  int                                   id,
  file_history&                         history,
  std::function<void(wxUpdateUIEvent&)> ui)
  : m_id(id)
  , m_type(HISTORY)
  , m_history(&history)
{
  if (auto* frame = dynamic_cast<managed_frame*>(wxTheApp->GetTopWindow());
      frame != nullptr && ui != nullptr)
  {
    frame->Bind(wxEVT_UPDATE_UI, ui, m_id);
  }
}

wex::menu_item::menu_item(const managed_frame* frame)
  : m_frame(frame)
  , m_type(PANES)
{
}

void wex::menu_item::append(wex::menu* menu) const
{
  switch (m_type)
  {
    case CHECK:
      menu->AppendCheckItem(m_id, m_name, m_help_text);
      break;

    case HISTORY:
      m_history->use_menu(m_id, menu);
      break;

    case MENU:
    {
      auto* item = new wxMenuItem(menu, m_id, m_name, m_help_text);

      if (const stockart art(m_id); art.get_bitmap().IsOk())
      {
        item->SetBitmap(art.get_bitmap(
          wxART_MENU,
          wxArtProvider::GetSizeHint(wxART_MENU, true)));
      }
      else if (!m_artid.empty())
      {
        if (const wxBitmap bitmap(wxArtProvider::GetBitmap(
              m_artid,
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
      menu->AppendRadioItem(m_id, m_name, m_help_text);
      break;

    case SUBMENU:
      if (m_id == wxID_ANY)
      {
        menu->AppendSubMenu(m_menu, m_name, m_help_text);
      }
      else
      {
        // This one is deprecated, but is necessary if
        // we have an explicit itemid.
        menu->Append(m_id, m_name, m_menu, m_help_text);
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
    if (it.first.first == "PROCESS" && process::get_shell() == nullptr)
    {
      continue;
    }

    menu->append({{it.second, it.first.second, CHECK}});
  }
}

void wex::menu_item::append_vcs(wex::menu* menu) const
{
  if (!m_path.file_exists())
  {
    if (m_path.dir_exists())
    {
      const wex::vcs vcs({m_path.string()});

      vcs.entry().build_menu(ID_VCS_LOWEST + 1, menu);
    }
    else
    {
      wex::vcs vcs;

      if (vcs.set_entry_from_base(m_modal ? wxTheApp->GetTopWindow() : nullptr))
      {
        vcs.entry().build_menu(ID_VCS_LOWEST + 1, menu);
      }
    }
  }
  else
  {
    auto* vcsmenu = new wex::menu(menu->style());

    if (const wex::vcs vcs({m_path.string()});
        vcs.entry().build_menu(ID_EDIT_VCS_LOWEST + 1, vcsmenu))
    {
      menu->append({{vcsmenu, vcs.entry().name()}});
    }
  }
}
