////////////////////////////////////////////////////////////////////////////////
// Name:      filehistory.cpp
// Purpose:   Implementation of wxExFileHistory class methods
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/generic/dirctrlg.h> // for wxTheFileIconsTable
#include <wx/imaglist.h>
#include <wx/extension/filehistory.h>
#include <wx/extension/filename.h>
#include <wx/extension/util.h>

wxExFileHistory::wxExFileHistory(
  size_t maxFiles, wxWindowID idBase, const std::string& key)
  : wxFileHistory(maxFiles, idBase)
  , m_Key(key)
{
  // There is only support for one history in the config.
  // We use file history for this, so update project history ourselves.
  // The order should be inverted, as the last one added is the most recent used.
  if (!key.empty())
  {
    for (int i = GetMaxFiles() - 1 ; i >=0 ; i--)
    {
      AddFileToHistory(
        wxConfigBase::Get()->Read(wxString::Format("%s/%d", key.c_str(), i)));
    }
  }
}

void wxExFileHistory::AddFileToHistory(const wxString& file)
{
  if (!file.empty() && GetMaxFiles() > 0 && wxFileExists(file))
  {
    wxFileHistory::AddFileToHistory(file);
  }
}

void wxExFileHistory::Clear()
{
  if (GetCount() > 0)
  {
    for (int i = GetCount() - 1; i >= 0; i--)
    {
      RemoveFileFromHistory(i);
    }
  }
}

wxString wxExFileHistory::GetHistoryFile(size_t index) const
{
  if (GetCount() > 0 && (int)index < GetMaxFiles())
  {
    const wxString file(wxFileHistory::GetHistoryFile(index));

    if (!wxFileExists(file))
    {
      const_cast< wxExFileHistory * >( this )->RemoveFileFromHistory(index);
      wxLogStatus(_("Removed not existing file: %s from history"), 
        file.c_str());
    }
    else
    {
      return wxFileHistory::GetHistoryFile(index);
    }
  }
  
  return wxEmptyString;
}
    
std::vector<std::string> wxExFileHistory::GetVector(size_t count) const
{
  std::vector<std::string> v;
  
  for (size_t i = 0; i < count && i < GetCount(); i++)
  {
    v.emplace_back(GetHistoryFile(i).ToStdString());
  }  
  
  return v;
}
  
void wxExFileHistory::PopupMenu(wxWindow* win,
  int clear_id, const wxPoint& pos) const
{
  wxMenu* menu = new wxMenu();

  for (size_t i = 0; i < GetCount(); i++)
  {
    const wxExFileName file(GetHistoryFile(i).ToStdString());
    
    if (file.FileExists())
    {
      wxMenuItem* item = new wxMenuItem(
        menu, 
        GetBaseId() + i, 
        file.GetFullName());

      item->SetBitmap(wxTheFileIconsTable->GetSmallImageList()->GetBitmap(
        wxExGetIconID(file)));
    
      menu->Append(item);
    }
  }
  
  if (menu->GetMenuItemCount() > 0)
  {
    menu->AppendSeparator();
    menu->Append(clear_id, wxGetStockLabel(wxID_CLEAR));
      
    win->PopupMenu(menu, pos);
  }
    
  delete menu;
}

void wxExFileHistory::Save()
{
  if (m_Key.empty())
  {
    wxFileHistory::Save(*wxConfigBase::Get());
  }
  else
  {
    if (GetCount() > 0)
    {
      wxConfigBase::Get()->DeleteGroup(m_Key);
      
      for (size_t i = 0; i < GetCount(); i++)
      {
        wxConfigBase::Get()->Write(
          wxString::Format("%s/%lu", m_Key, i),
          GetHistoryFile(i));
      }
    }
  }
}
  
void wxExFileHistory::UseMenu(wxWindowID id, wxMenu* menu)
{
  wxMenu* submenu = new wxMenu;
  menu->Append(id, _("Open &Recent"), submenu);
  wxFileHistory::UseMenu(submenu);

  if (m_Key.empty())
  {
    // We can load file history now.
    Load(*wxConfigBase::Get());
  }
  else
  {
    AddFilesToMenu();
  }
}
