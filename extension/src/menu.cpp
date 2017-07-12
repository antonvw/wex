////////////////////////////////////////////////////////////////////////////////
// Name:      menu.cpp
// Purpose:   Implementation of wxExMenu class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
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

wxExMenu::wxExMenu(const wxString& title, long style)
  : wxMenu(title)
  , m_Style(style)
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
  else
  {
    const wxExStockArt art(id);

    if (art.GetBitmap().IsOk())
    {
      item->SetBitmap(art.GetBitmap(
        wxART_MENU, 
        wxArtProvider::GetSizeHint(wxART_MENU, true)));
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
  Append(wxID_PRINT_SETUP, wxString(wxExEllipsed(_("Page &Setup"))));
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
  if (wxExLexers::Get()->GetLexers().empty())
  {
    return false;
  }

  wxExMenu* menuTool = new wxExMenu(m_Style);

  for (const auto& it : wxExTool().GetToolInfo())
  {
    if (!it.second.GetText().empty())
    {
      menuTool->Append(
        it.first, 
        it.second.GetText(), 
        it.second.GetHelpText());
    }
  }

  AppendSubMenu(menuTool, _("&Tools"), wxEmptyString, itemid);

  return true;
}

bool wxExMenu::AppendVCS(const wxExPath& filename, bool show_modal)
{
  if (!filename.GetStat().IsOk())
  {
    wxExVCS vcs;
       
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
    wxExMenu* vcsmenu = new wxExMenu;
  
    const wxExVCS vcs({filename.Path().string()});

    if (vcs.GetEntry().BuildMenu(ID_EDIT_VCS_LOWEST + 1, vcsmenu))
    { 
      AppendSubMenu(vcsmenu, vcs.GetEntry().GetName());
      return true;
    }
  }
  
  return false;
}
#endif // wxUSE_GUI
