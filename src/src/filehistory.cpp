////////////////////////////////////////////////////////////////////////////////
// Name:      file_history.cpp
// Purpose:   Implementation of wex::file_history class methods
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <filesystem>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/filehistory.h>
#include <wx/generic/dirctrlg.h> // for wxTheFileIconsTable
#include <wx/imaglist.h>
#include <wex/filehistory.h>
#include <wex/config.h>
#include <wex/menu.h>
#include <wex/path.h>
#include <wex/util.h>

namespace wex
{
  class file_history_imp : public wxFileHistory
  {
  public:
    file_history_imp(
      size_t maxFiles = 9, 
      wxWindowID idBase = wxID_FILE1)
      : wxFileHistory(maxFiles, idBase) {;};

    virtual void AddFileToHistory(const wxString& file) override;
    virtual wxString GetHistoryFile(size_t index = 0) const override;
  };
};

wex::file_history::file_history(
  size_t maxFiles, wxWindowID idBase, const std::string& key)
  : m_History(new file_history_imp(maxFiles, idBase))
  , m_Key(key)
{
  // The order should be inverted, as the last one added is the most recent used.
  if (!key.empty())
  {
    for (int i = get_max_files() - 1 ; i >=0 ; i--)
    {
      m_History->AddFileToHistory(
        config(key + "/" + std::to_string(i)).get());
    }
  }
}

wex::file_history::~file_history()
{
  delete m_History;
}
  
void wex::file_history::add(const path& p)
{
  if (p.file_exists())
  {
    m_History->AddFileToHistory(p.data().string());
  }
}

void wex::file_history::clear()
{
  if (size() > 0)
  {
    for (int i = size() - 1; i >= 0; i--)
    {
      m_History->RemoveFileFromHistory(i);
    }
  }
}

wxWindowID wex::file_history::get_base_id() const
{
  return m_History->GetBaseId();
}
  
size_t wex::file_history::size() const
{
  return m_History->GetCount();
}
  
int wex::file_history::get_max_files() const
{
  return m_History->GetMaxFiles();
}

wex::path wex::file_history::get_history_file(size_t index) const
{
  return path(m_History->GetHistoryFile(index).ToStdString());
}
    
std::vector<wex::path> wex::file_history::get_history_files(size_t count) const
{
  std::vector<path> v;
  
  for (size_t i = 0; i < count && i < size(); i++)
  {
    v.emplace_back(get_history_file(i));
  }  
  
  return v;
}
  
void wex::file_history::popup_menu(wxWindow* win,
  int clear_id, const wxPoint& pos) const
{
  auto* menu = new wex::menu();

  for (size_t i = 0; i < size(); i++)
  {
    if (const wex::path file(get_history_file(i)); file.file_exists())
    {
      auto* item = new wxMenuItem(
        menu, 
        get_base_id() + i, 
        file.fullname());

      item->SetBitmap(wxTheFileIconsTable->GetSmallImageList()->GetBitmap(
        get_iconid(file)));
    
      menu->Append(item);
    }
  }
  
  if (menu->GetMenuItemCount() > 0)
  {
    menu->append_separator();
    
    if (clear_id != -1)
    {
      menu->append(clear_id, wxGetStockLabel(wxID_CLEAR));
    }
      
    win->PopupMenu(menu, pos);
  }
    
  delete menu;
}

void wex::file_history::save()
{
  if (m_Key.empty())
  {
    m_History->Save(*config::wx_config());
  }
  else
  {
    if (size() > 0)
    {
      config(m_Key).erase();
      
      for (size_t i = 0; i < size(); i++)
      {
        config(m_Key + "/" + std::to_string(i)).set(
          m_History->GetHistoryFile(i).ToStdString());
      }
    }
  }
}
  
void wex::file_history::use_menu(wxWindowID id, wxMenu* menu)
{
  wxMenu* submenu = new wxMenu;
  menu->Append(id, _("Open &Recent"), submenu);
  m_History->UseMenu(submenu);

  if (m_Key.empty())
  {
    // We can load file history now.
    m_History->Load(*config::wx_config());
  }
  else
  {
    m_History->AddFilesToMenu();
  }
}

// Implementation

void wex::file_history_imp::AddFileToHistory(const wxString& file)
{
  if (!file.empty() && GetMaxFiles() > 0)
  {
    wxFileHistory::AddFileToHistory(file);
  }
}

wxString wex::file_history_imp::GetHistoryFile(size_t index) const
{
  if (GetCount() > 0 && (int)index < GetMaxFiles())
  {
    bool error = false;
    std::string file;

    try
    {
      file = wxFileHistory::GetHistoryFile(index);

      if (!std::filesystem::is_regular_file(file))
      {
        error = true;
      }
      else
      {
        return wxFileHistory::GetHistoryFile(index);
      }
    }
    catch (const std::exception& )
    {
      error = true;
    }

    if (error)
    {
      const_cast< file_history_imp * >( this )->RemoveFileFromHistory(index);
      log::status(_("Removed not existing file")) << file << "from history";
    }
  }
  
  return wxEmptyString;
}
