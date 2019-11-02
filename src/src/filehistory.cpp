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
      wxWindowID idBase = wxID_FILE1,
      const std::string& key = "xxxxx")
      : wxFileHistory(maxFiles, idBase)
      , m_key(key.empty() ? "RecentFiles": key)
      , m_history(config(m_key).get(std::list < std::string >{})) 
      {
        // The order should be inverted, as the last one added is the most recent used.
        for (
          auto it = m_history.rbegin(); it != m_history.rend(); ++it)
        {
          AddFileToHistory(*it);
        }
      }
    
    void AddFileToHistory(const wxString& file) override;
    wxString GetHistoryFile(size_t index = 0) const override;

    void add(const path& p)
    {
      if (p.file_exists())
      {
        m_history.remove(p.string());
        m_history.push_front(p.string());
        AddFileToHistory(p.string());
      }
    }

    const auto& history() const {return m_key;};

    void save() {config(m_key).set(m_history);};
  private:
    const std::string m_key;
    std::list < std::string> m_history;
  };
};

wex::file_history::file_history(
  size_t maxFiles, wxWindowID idBase, const std::string& key)
  : m_history(new file_history_imp(maxFiles, idBase, key))
{
}

wex::file_history::~file_history()
{
  delete m_history;
}
  
void wex::file_history::add(const path& p)
{
  m_history->add(p);
}

void wex::file_history::clear()
{
  if (size() > 0)
  {
    for (int i = size() - 1; i >= 0; i--)
    {
      m_history->RemoveFileFromHistory(i);
    }
  }
}

wxWindowID wex::file_history::get_base_id() const
{
  return m_history->GetBaseId();
}
  
int wex::file_history::get_max_files() const
{
  return m_history->GetMaxFiles();
}

wex::path wex::file_history::get_history_file(size_t index) const
{
  try
  {
    return path(m_history->GetHistoryFile(index).ToStdString());
  }
  catch (const std::exception& e)
  {
    wex::log(e) << "get_history_file:" << index;
    return path();
  }
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
  m_history->save();
}

size_t wex::file_history::size() const
{
  return m_history->GetCount();
}
  
void wex::file_history::use_menu(wxWindowID id, wxMenu* menu)
{
  wxMenu* submenu = new wxMenu;
  menu->Append(id, _("Open &Recent"), submenu);

  m_history->UseMenu(submenu);
  m_history->AddFilesToMenu();
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
      const_cast< file_history_imp * >( this )->m_history.remove(file);
      const_cast< file_history_imp * >( this )->RemoveFileFromHistory(index);
      log::status(_("Removed not existing file")) << file << "from history";
    }
  }
  
  return wxEmptyString;
}
