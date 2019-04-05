////////////////////////////////////////////////////////////////////////////////
// Name:      menu.cpp
// Purpose:   Implementation of wex::menu class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/menu.h>
#include <wex/art.h>
#include <wex/lexers.h>
#include <wex/tool.h>
#include <wex/util.h> // for wex::ellipsed
#include <wex/vcs.h>

wex::menu::menu(menu_t style)
  : m_Style(style)
{
}

wex::menu::menu(const std::string& title, menu_t style)
  : wxMenu(title)
  , m_Style(style)
{
}
  
wxMenuItem* wex::menu::append(
  int id,
  const std::string& name,
  const std::string& helptext,
  const wxArtID& artid)
{
  auto* item = new wxMenuItem(this, id, name, helptext);

  if (const stockart art(id); art.get_bitmap().IsOk())
  {
    item->SetBitmap(art.get_bitmap(
      wxART_MENU, 
      wxArtProvider::GetSizeHint(wxART_MENU, true)));
  }
  else if (!artid.empty())
  {
    if (const wxBitmap bitmap(wxArtProvider::GetBitmap(
        artid, 
        wxART_MENU, 
        wxArtProvider::GetSizeHint(wxART_MENU, true)));
      bitmap.IsOk())
    {
      item->SetBitmap(bitmap);
    }
  }

  return Append(item);
}

void wex::menu::append_edit(bool add_invert)
{
  if (!m_Style[IS_READ_ONLY] && m_Style[IS_SELECTED])
  {
    append(wxID_CUT);
  }

  if (m_Style[IS_SELECTED])
  {
    append(wxID_COPY);
  }

  if (!m_Style[IS_READ_ONLY] && m_Style[CAN_PASTE])
  {
    append(wxID_PASTE);
  }

  if (!m_Style[IS_SELECTED] && !m_Style[IS_EMPTY])
  {
    append(wxID_SELECTALL);
  }
  else
  {
    if (add_invert && !m_Style[IS_EMPTY])
    {
      append(ID_EDIT_SELECT_NONE, _("&Deselect All"));
    }
  }

  if (m_Style[ALLOW_CLEAR])
  {
    append(wxID_CLEAR);
  }

  if (add_invert && !m_Style[IS_EMPTY])
  {
    append(ID_EDIT_SELECT_INVERT, _("&Invert"));
  }

  if (!m_Style[IS_READ_ONLY] &&
       m_Style[IS_SELECTED] &&
      !m_Style[IS_EMPTY])
  {
    append(wxID_DELETE);
  }
}

void wex::menu::append_print()
{
  append(wxID_PRINT_SETUP, ellipsed(_("Page &Setup")));
  append(wxID_PREVIEW);
  append(wxID_PRINT);
}

void wex::menu::append_separator()
{
  if (
    GetMenuItemCount() == 0 ||
    FindItemByPosition(GetMenuItemCount() - 1)->IsSeparator())
  {
    return;
  }

  AppendSeparator();
}

void wex::menu::append_submenu(
  wxMenu *submenu,
  const std::string& text,
  const std::string& help,
  int itemid)
{
  if (itemid == wxID_ANY)
  {
    AppendSubMenu(submenu, text, help);
  }
  else
  {
    // This one is deprecated, but is necessary if
    // we have an explicit itemid.
    Append(itemid, text, submenu, help);
  }
}

bool wex::menu::append_tools(int itemid)
{
  if (lexers::get()->get_lexers().empty())
  {
    return false;
  }

  auto* menuTool = new wex::menu(m_Style);

  for (const auto& it : tool().get_tool_info())
  {
    if (!it.second.text().empty())
    {
      menuTool->append(
        it.first, 
        it.second.text(), 
        it.second.help_text());
    }
  }

  append_submenu(menuTool, _("&Tools"), std::string(), itemid);

  return true;
}

bool wex::menu::append_vcs(const path& filename, bool show_modal)
{
  if (!filename.stat().is_ok())
  {
    wex::vcs vcs;
       
    if (vcs.set_entry_from_base(
      show_modal ? wxTheApp->GetTopWindow(): nullptr))
    {
      return vcs.entry().build_menu(
        ID_VCS_LOWEST + 1, 
        this, 
        false) > 0; // no popup
    }
  }
  else
  {
    auto* vcsmenu = new wex::menu;

    if (const wex::vcs vcs({filename.data().string()});
      vcs.entry().build_menu(ID_EDIT_VCS_LOWEST + 1, vcsmenu))
    { 
      append_submenu(vcsmenu, vcs.entry().name());
      return true;
    }
  }
  
  return false;
}
