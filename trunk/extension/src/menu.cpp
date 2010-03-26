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
  , m_ItemsAppended(0)
  , m_IsSeparator(false)
  , m_MenuVCSFilled(false)
{
}

wxExMenu::wxExMenu(const wxExMenu& menu)
  : m_Style(menu.m_Style)
  , m_ItemsAppended(menu.m_ItemsAppended)
  , m_IsSeparator(menu.m_IsSeparator)
  , m_MenuVCSFilled(menu.m_MenuVCSFilled)
{
}

wxMenuItem* wxExMenu::Append(int id)
{
  m_ItemsAppended++;
  m_IsSeparator = false;

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
  m_ItemsAppended++;
  m_IsSeparator = false;

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
  if (m_ItemsAppended == 0 || m_IsSeparator) return;

  wxMenu::AppendSeparator();

  m_IsSeparator = true;
}

void wxExMenu::AppendSubMenu(
  wxMenu *submenu,
  const wxString& text,
  const wxString& help,
  int itemid)
{
  m_ItemsAppended++; // count submenu as one
  m_IsSeparator = false;

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

void wxExMenu::AppendTools(int itemid)
{
  if (wxExLexers::Get()->Count() == 0)
  {
    return;
  }

  wxExMenu* menuTool = new wxExMenu(*this);

  for (
    std::map <int, wxExToolInfo>::const_iterator it = 
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
}

// This is the VCS submenu, as present on a popup.
// Therefore it is build when clicking, and does not
// need to be destroyed an old one.
void wxExMenu::AppendVCS()
{
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

  AppendSeparator();
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
      DestroyVCS(id);
    }
  }

  if (fill)
  {
    const int vcs_offset_id = ID_VCS_LOWEST;

    AppendVCS(vcs_offset_id + wxExVCS::VCS_STAT);
    AppendVCS(vcs_offset_id + wxExVCS::VCS_INFO);
    AppendVCS(vcs_offset_id + wxExVCS::VCS_LOG);
    AppendVCS(vcs_offset_id + wxExVCS::VCS_LS);
    AppendVCS(vcs_offset_id + wxExVCS::VCS_DIFF);
    AppendVCS(vcs_offset_id + wxExVCS::VCS_HELP);
    AppendSeparator();
    AppendVCS(vcs_offset_id + wxExVCS::VCS_UPDATE);
    AppendVCS(vcs_offset_id + wxExVCS::VCS_COMMIT);
    AppendSeparator();
    AppendVCS(vcs_offset_id + wxExVCS::VCS_ADD);
  }

  m_MenuVCSFilled = fill;
}

void wxExMenu::DestroyVCS(int id)
{
  // When using only Destroy, and the item does not exist,
  // an assert happens.

  if (FindItem(id))
  {
    Destroy(id);
  }
}

#endif // wxUSE_GUI
