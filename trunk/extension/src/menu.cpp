/******************************************************************************\
* File:          menu.cpp
* Purpose:       Implementation of wxExMenu class
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <map>
#include <wx/extension/menu.h>
#include <wx/extension/art.h>
#include <wx/extension/lexers.h>
#include <wx/extension/tool.h>
#include <wx/extension/util.h> // for wxExEllipsed
#include <wx/extension/vcs.h>

#if wxUSE_GUI

wxExMenu::wxExMenu(long style)
  : m_Style(style)
  , m_MenuVCSFilled(false)
{
}

wxExMenu::wxExMenu(const wxExMenu& menu)
  : m_Style(menu.m_Style)
  , m_MenuVCSFilled(menu.m_MenuVCSFilled)
{
}

wxMenuItem* wxExMenu::Append(int id)
{
  // Using wxMenu::Append(id)
  // also appends the stock item,
  // but does not add the bitmap.

  wxMenuItem* item = new wxMenuItem(this, id);

  const wxExStockArt art(id);

  if (art.GetBitmap().IsOk())
  {
    item->SetBitmap(art.GetBitmap(
      wxART_MENU, 
      wxArtProvider::GetSizeHint(wxART_MENU, true)));
  }

  return wxMenu::Append(item);
}

wxMenuItem* wxExMenu::Append(
  int id,
  const wxString& name,
  const wxString& helptext,
  const wxArtID& artid)
{
  wxMenuItem* item = new wxMenuItem(this, id, name, helptext);

  if (!artid.empty())
  {
    const wxBitmap bitmap = 
      wxArtProvider::GetBitmap(
        artid, 
        wxART_MENU, 
        wxArtProvider::GetSizeHint(wxART_MENU, true));

    if (bitmap.IsOk())
    {
      item->SetBitmap(bitmap);
    }
  }

  return wxMenu::Append(item);
}

void wxExMenu::AppendBars()
{
  AppendCheckItem(ID_VIEW_MENUBAR, _("&Menubar"));
  AppendCheckItem(ID_VIEW_STATUSBAR, _("&Statusbar"));
  AppendCheckItem(ID_VIEW_TOOLBAR, _("&Toolbar"));
  AppendCheckItem(ID_VIEW_FINDBAR, _("&Findbar"));
}

void wxExMenu::AppendEdit(bool add_invert)
{
  if (!(m_Style & MENU_IS_READ_ONLY) &&
       (m_Style & MENU_IS_SELECTED))
  {
    Append(wxID_CUT);
  }

  if (m_Style & MENU_IS_SELECTED)
  {
    Append(wxID_COPY);
  }

  if (!(m_Style & MENU_IS_READ_ONLY) &&
       (m_Style & MENU_CAN_PASTE))
  {
    Append(wxID_PASTE);
  }

  if (!(m_Style & MENU_IS_SELECTED) &&
      !(m_Style & MENU_IS_EMPTY))
  {
    Append(wxID_SELECTALL);
  }
  else
  {
    if (add_invert && !(m_Style & MENU_IS_EMPTY))
    {
      Append(ID_EDIT_SELECT_NONE, _("&Deselect All"));
    }
  }

  if (m_Style & MENU_ALLOW_CLEAR)
  {
    Append(wxID_CLEAR);
  }

  if (add_invert && !(m_Style & MENU_IS_EMPTY))
  {
    Append(ID_EDIT_SELECT_INVERT, _("&Invert"));
  }

  if (!(m_Style & MENU_IS_READ_ONLY) &&
       (m_Style & MENU_IS_SELECTED) &&
      !(m_Style & MENU_IS_EMPTY))
  {
    Append(wxID_DELETE);
  }
}

void wxExMenu::AppendPrint()
{
  Append(wxID_PRINT_SETUP, wxExEllipsed(_("Page &Setup")));
  Append(wxID_PREVIEW);
  Append(wxID_PRINT);
}

void wxExMenu::AppendSeparator()
{
  if (
    GetMenuItemCount() == 0 ||
    FindItemByPosition(GetMenuItemCount() - 1)->IsSeparator())
  {
    return;
  }

  wxMenu::AppendSeparator();
}

void wxExMenu::AppendSubMenu(
  wxMenu *submenu,
  const wxString& text,
  const wxString& help,
  int itemid)
{
  if (itemid == wxID_ANY)
  {
    wxMenu::AppendSubMenu(submenu, text, help);
  }
  else
  {
    // This one is deprecated, but is necessary if
    // we have an explicit itemid.
    wxMenu::Append(itemid, text, submenu, help);
  }
}

bool wxExMenu::AppendTools(int itemid)
{
  if (wxExLexers::Get()->Count() == 0)
  {
    return false;
  }

  wxExMenu* menuTool = new wxExMenu(*this);

  for (
    auto it = 
      wxExTool::Get()->GetToolInfo().begin();
    it != wxExTool::Get()->GetToolInfo().end();
    ++it)
  {
    if (!it->second.GetText().empty())
    {
      const bool vcs_type = wxExTool(it->first).IsRCSType();

      if ((vcs_type && !wxExVCS::Get()->Use()) || !vcs_type)
      {
        menuTool->Append(
          it->first, 
          it->second.GetText(), 
          it->second.GetHelpText());
      }
    }
  }

  AppendSubMenu(menuTool, _("&Tools"), wxEmptyString, itemid);

  return true;
}

// This is the VCS submenu, as present on a popup.
// Therefore it is build when clicking, and does not
// need to be destroyed an old one.
void wxExMenu::AppendVCS()
{
  if (!wxExVCS::Get()->Use())
  {
    return;
  }

  const int vcs_offset_id = ID_EDIT_VCS_LOWEST;

  wxExMenu* vcsmenu = new wxExMenu;
  vcsmenu->AppendVCS(vcs_offset_id + wxExVCS::VCS_LOG);
  vcsmenu->AppendVCS(vcs_offset_id + wxExVCS::VCS_STAT);
  vcsmenu->AppendVCS(vcs_offset_id + wxExVCS::VCS_SHOW);
  vcsmenu->AppendVCS(vcs_offset_id + wxExVCS::VCS_DIFF);
  vcsmenu->AppendSeparator();
  vcsmenu->AppendVCS(vcs_offset_id + wxExVCS::VCS_COMMIT);
  vcsmenu->AppendSeparator();
  vcsmenu->AppendVCS(vcs_offset_id + wxExVCS::VCS_CAT);
  vcsmenu->AppendVCS(vcs_offset_id + wxExVCS::VCS_BLAME);
  vcsmenu->AppendSeparator();
  vcsmenu->AppendVCS(vcs_offset_id + wxExVCS::VCS_PROPLIST);
  vcsmenu->AppendVCS(vcs_offset_id + wxExVCS::VCS_PROPSET);
  vcsmenu->AppendSeparator();
  vcsmenu->AppendVCS(vcs_offset_id + wxExVCS::VCS_REVERT);
  vcsmenu->AppendSeparator();
  vcsmenu->AppendVCS(vcs_offset_id + wxExVCS::VCS_ADD);

  AppendSubMenu(vcsmenu, "&VCS");
}

void wxExMenu::AppendVCS(int id)
{
  const wxString command = wxExVCS(id).GetCommandString();

  if (command.empty())
  {
    return;
  }

  const wxString text(wxExEllipsed("&" + command));

  Append(id, text);
}

// This is the general VCS menu, it is in the main menu,
// and because contents depends on actual VCS used,
// it is rebuild after change of VCS system.
void wxExMenu::BuildVCS(bool fill)
{
  if (m_MenuVCSFilled)
  {
    wxMenuItem* item;

    while ((item = FindItem(wxID_SEPARATOR)) != NULL)
    {
      Destroy(item);
    }

    for (int id = ID_VCS_LOWEST + 1; id < ID_VCS_HIGHEST; id++)
    {
      // When using only Destroy, and the item does not exist,
      // an assert happens.
      if (FindItem(id) != NULL)
      {
        Destroy(id);
      }
    }
  }

  if (fill)
  {
    const int vcs_offset_id = ID_VCS_LOWEST;

    AppendVCS(vcs_offset_id + wxExVCS::VCS_STAT);
    AppendVCS(vcs_offset_id + wxExVCS::VCS_INFO);
    AppendVCS(vcs_offset_id + wxExVCS::VCS_LOG);
    AppendVCS(vcs_offset_id + wxExVCS::VCS_LS);
    // The VCS_DIFF is not used, as it requires diff between files,
    // and not a folder.
    //AppendVCS(vcs_offset_id + wxExVCS::VCS_DIFF);
    AppendVCS(vcs_offset_id + wxExVCS::VCS_HELP);
    AppendSeparator();
    AppendVCS(vcs_offset_id + wxExVCS::VCS_UPDATE);
    AppendVCS(vcs_offset_id + wxExVCS::VCS_COMMIT);
    AppendVCS(vcs_offset_id + wxExVCS::VCS_PUSH);
    AppendSeparator();
    AppendVCS(vcs_offset_id + wxExVCS::VCS_ADD);
  }

  m_MenuVCSFilled = fill;
}
#endif // wxUSE_GUI
