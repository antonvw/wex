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
#include <wx/extension/tool.h>
#include <wx/extension/util.h> // for wxExEllipsed

#if wxUSE_GUI

wxExMenu::wxExMenu(long style)
  : m_Style(style)
  , m_ItemsAppended(0)
  , m_IsSeparator(false)
{
}

wxExMenu::wxExMenu(const wxExMenu& menu)
  : m_Style(menu.m_Style)
  , m_ItemsAppended(menu.m_ItemsAppended)
  , m_IsSeparator(menu.m_IsSeparator)
{
}

wxMenuItem* wxExMenu::Append(int id)
{
  m_ItemsAppended++;
  m_IsSeparator = false;

  const wxExStockArt art(id);

  wxMenuItem* item = new wxMenuItem(this, id, wxGetStockLabel(id));

  if (art.GetBitmap().IsOk())
  {
    item->SetBitmap(art.GetBitmap(wxART_MENU));
  }

  return wxMenu::Append(item);
}

wxMenuItem* wxExMenu::Append(
  int id,
  const wxString& name,
  const wxString& helptext,
  wxArtID artid)
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
  const wxString& help)
{
  m_ItemsAppended++; // count submenu as one
  m_IsSeparator = false;
  wxMenu::AppendSubMenu(submenu, text, help);
}

void wxExMenu::AppendSVN()
{
  wxMenu* svnmenu = new wxMenu;
  svnmenu->Append(ID_EDIT_SVN_LOG, wxExEllipsed("&Log"));
  svnmenu->Append(ID_EDIT_SVN_STAT, wxExEllipsed("&Stat"));
  svnmenu->Append(ID_EDIT_SVN_DIFF, wxExEllipsed("&Diff"));
  svnmenu->AppendSeparator();
  svnmenu->Append(ID_EDIT_SVN_COMMIT, wxExEllipsed("&Commit"));
  svnmenu->AppendSeparator();
  svnmenu->Append(ID_EDIT_SVN_CAT, wxExEllipsed("Ca&t"));
  svnmenu->Append(ID_EDIT_SVN_BLAME, wxExEllipsed("&Blame"));
  svnmenu->AppendSeparator();
  svnmenu->Append(ID_EDIT_SVN_PROPLIST, wxExEllipsed("&Proplist"));
  svnmenu->Append(ID_EDIT_SVN_PROPSET, wxExEllipsed("&Propset"));
  svnmenu->AppendSeparator();
  svnmenu->Append(ID_EDIT_SVN_REVERT, wxExEllipsed("&Revert"));
  svnmenu->AppendSeparator();
  svnmenu->Append(ID_EDIT_SVN_ADD, wxExEllipsed("&Add"));

  AppendSeparator();
  AppendSubMenu(svnmenu, "&SVN");
}

void wxExMenu::AppendTools()
{
  wxExMenu* menuTool = new wxExMenu(*this);

  for (
    std::map <int, wxExToolInfo>::const_iterator it = 
      wxExTool::Get()->GetToolInfo().begin();
    it != wxExTool::Get()->GetToolInfo().end();
    ++it)
  {
    if (!it->second.GetText().empty())
    {
      menuTool->Append(
        it->first, 
        it->second.GetText(), 
        it->second.GetHelpText());
    }
  }

  AppendSubMenu(menuTool, _("&Tools"));
}

#endif // wxUSE_GUI
