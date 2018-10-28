////////////////////////////////////////////////////////////////////////////////
// Name:      menu.cpp
// Purpose:   Implementation of wex::menu class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <map>
#include <wex/menu.h>
#include <wex/art.h>
#include <wex/lexers.h>
#include <wex/tool.h>
#include <wex/util.h> // for wex::ellipsed
#include <wex/vcs.h>

wex::menu::menu(long style)
  : m_Style(style)
{
}

wex::menu::menu(const std::string& title, long style)
  : wxMenu(title)
  , m_Style(style)
{
}
  
wxMenuItem* wex::menu::Append(
  int id,
  const std::string& name,
  const std::string& helptext,
  const wxArtID& artid)
{
  wxMenuItem* item = new wxMenuItem(this, id, name, helptext);

  const stockart art(id);

  if (art.GetBitmap().IsOk())
  {
    item->SetBitmap(art.GetBitmap(
      wxART_MENU, 
      wxArtProvider::GetSizeHint(wxART_MENU, true)));
  }
  else if (!artid.empty())
  {
    const wxBitmap bitmap(
      wxArtProvider::GetBitmap(
        artid, 
        wxART_MENU, 
        wxArtProvider::GetSizeHint(wxART_MENU, true)));

    if (bitmap.IsOk())
    {
      item->SetBitmap(bitmap);
    }
  }

  return wxMenu::Append(item);
}

void wex::menu::append_edit(bool add_invert)
{
  if (!(m_Style & IS_READ_ONLY) &&
       (m_Style & IS_SELECTED))
  {
    Append(wxID_CUT);
  }

  if (m_Style & IS_SELECTED)
  {
    Append(wxID_COPY);
  }

  if (!(m_Style & IS_READ_ONLY) &&
       (m_Style & CAN_PASTE))
  {
    Append(wxID_PASTE);
  }

  if (!(m_Style & IS_SELECTED) &&
      !(m_Style & IS_EMPTY))
  {
    Append(wxID_SELECTALL);
  }
  else
  {
    if (add_invert && !(m_Style & IS_EMPTY))
    {
      Append(ID_EDIT_SELECT_NONE, _("&Deselect All"));
    }
  }

  if (m_Style & ALLOW_CLEAR)
  {
    Append(wxID_CLEAR);
  }

  if (add_invert && !(m_Style & IS_EMPTY))
  {
    Append(ID_EDIT_SELECT_INVERT, _("&Invert"));
  }

  if (!(m_Style & IS_READ_ONLY) &&
       (m_Style & IS_SELECTED) &&
      !(m_Style & IS_EMPTY))
  {
    Append(wxID_DELETE);
  }
}

void wex::menu::append_print()
{
  Append(wxID_PRINT_SETUP, ellipsed(_("Page &Setup")));
  Append(wxID_PREVIEW);
  Append(wxID_PRINT);
}

void wex::menu::append_separator()
{
  if (
    GetMenuItemCount() == 0 ||
    FindItemByPosition(GetMenuItemCount() - 1)->IsSeparator())
  {
    return;
  }

  wxMenu::AppendSeparator();
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
    wxMenu::Append(itemid, text, submenu, help);
  }
}

bool wex::menu::append_tools(int itemid)
{
  if (lexers::Get()->get().empty())
  {
    return false;
  }

  wex::menu* menuTool = new wex::menu(m_Style);

  for (const auto& it : tool().GetToolInfo())
  {
    if (!it.second.GetText().empty())
    {
      menuTool->Append(
        it.first, 
        it.second.GetText(), 
        it.second.GetHelpText());
    }
  }

  append_submenu(menuTool, _("&Tools"), std::string(), itemid);

  return true;
}

bool wex::menu::append_vcs(const path& filename, bool show_modal)
{
  if (!filename.GetStat().is_ok())
  {
    wex::vcs vcs;
       
    if (vcs.SetEntryFromBase(
      show_modal ? wxTheApp->GetTopWindow(): nullptr))
    {
      return vcs.GetEntry().BuildMenu(
        ID_VCS_LOWEST + 1, 
        this, 
        false) > 0; // no popup
    }
  }
  else
  {
    wex::menu* vcsmenu = new wex::menu;
  
    const wex::vcs vcs({filename.Path().string()});

    if (vcs.GetEntry().BuildMenu(ID_EDIT_VCS_LOWEST + 1, vcsmenu))
    { 
      append_submenu(vcsmenu, vcs.GetEntry().GetName());
      return true;
    }
  }
  
  return false;
}
