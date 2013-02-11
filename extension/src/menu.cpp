////////////////////////////////////////////////////////////////////////////////
// Name:      menu.cpp
// Purpose:   Implementation of wxExMenu class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2013 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

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
{
}

wxExMenu::wxExMenu(const wxExMenu& menu)
  : m_Style(menu.m_Style)
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
#ifdef __WXMSW__
  // See wxExFrame::Initialize
  AppendCheckItem(ID_VIEW_MENUBAR, _("&Menubar\tCtrl+I"));
#endif
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
  if (wxExLexers::Get()->GetCount() == 0)
  {
    return false;
  }

  wxExMenu* menuTool = new wxExMenu(*this);

  for (
#ifdef wxExUSE_CPP0X	
    auto it = 
      wxExTool().GetToolInfo().begin();
#else
    std::map < int, wxExToolInfo >::const_iterator it = 
      wxExTool().GetToolInfo().begin();
#endif	  
    it != wxExTool().GetToolInfo().end();
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

  AppendSubMenu(menuTool, _("&Tools"), wxEmptyString, itemid);

  return true;
}

bool wxExMenu::AppendVCS(const wxFileName& filename, bool show_modal)
{
  if (!filename.IsOk())
  {
    wxExVCS vcs;
       
    if (vcs.GetDir(
      show_modal ? wxTheApp->GetTopWindow(): NULL))
    {
      const int vcs_offset_id = ID_VCS_LOWEST + 1;
 
      return vcs.GetEntry().BuildMenu(
        vcs_offset_id, 
        this, 
        false) > 0;// no popup
    }
  }
  else
  {
    const int vcs_offset_id = ID_EDIT_VCS_LOWEST + 1;

    wxExMenu* vcsmenu = new wxExMenu;
  
    wxArrayString ar;
    ar.Add(filename.GetFullPath());
    const wxExVCS vcs(ar);

    if (vcs.GetEntry().BuildMenu(vcs_offset_id, vcsmenu))
    { 
      AppendSubMenu(vcsmenu, vcs.GetEntry().GetName());
      return true;
    }
  }
  
  return false;
}
#endif // wxUSE_GUI
