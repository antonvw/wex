////////////////////////////////////////////////////////////////////////////////
// Name:      file-history.cpp
// Purpose:   Implementation of wex::file_history class methods
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/config.h>
#include <wex/core/log.h>
#include <wex/core/path.h>
#include <wex/ui/file-history.h>
#include <wex/ui/menu.h>
#include <wx/filehistory.h>
#include <wx/stockitem.h>

#include <filesystem>
#include <ranges>

namespace wex
{
class file_history_imp : public wxFileHistory
{
public:
  explicit file_history_imp(
    size_t             maxFiles = 9,
    wxWindowID         idBase   = wxID_FILE1,
    const std::string& key      = std::string())
    : wxFileHistory(maxFiles, idBase)
    , m_key(key.empty() ? "recent.Files" : key)
    , m_contents(config(m_key).get(config::strings_t{}))
  {
  }

  bool append(const path& p)
  {
    if (!p.file_exists())
    {
      return false;
    }

    m_contents.remove(p.string());
    m_contents.push_front(p.string());
    AddFileToHistory(p.string());

    return true;
  }

  void clear() { m_contents.clear(); }

  const auto& contents() { return m_contents; }

  void save() { config(m_key).set(m_contents); }

  /// Override methods.

  void     AddFileToHistory(const wxString& file) override;
  wxString GetHistoryFile(size_t index = 0) const override;

private:
  const std::string         m_key;
  mutable config::strings_t m_contents;
};
}; // namespace wex

wex::file_history::file_history(
  size_t             maxFiles,
  wxWindowID         idBase,
  const std::string& key)
  : m_history(new file_history_imp(maxFiles, idBase, key))
{
  // The order should be inverted, as the last one added is the most recent
  // used.
  for (const auto& it : std::ranges::reverse_view(m_history->contents()))
  {
    m_history->AddFileToHistory(it);
  }
}

wex::file_history::~file_history()
{
  delete m_history;
}

const wex::path wex::file_history::operator[](size_t index) const
{
  try
  {
    return wex::path(m_history->GetHistoryFile(index).ToStdString());
  }
  catch (const std::exception& e)
  {
    wex::log(e) << "file_history::path:" << index;
    return path();
  }
}

bool wex::file_history::append(const wex::path& p)
{
  return m_history->append(p);
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

  m_history->clear();
}

bool wex::file_history::empty() const
{
  return size() == 0;
}

wxWindowID wex::file_history::get_base_id() const
{
  return m_history->GetBaseId();
}

std::vector<wex::path> wex::file_history::get_history_files(size_t count) const
{
  std::vector<wex::path> v;

  for (size_t i = 0; i < count && i < size(); i++)
  {
    v.emplace_back(operator[](i));
  }

  return v;
}

size_t wex::file_history::get_max_files() const
{
  return (size_t)m_history->GetMaxFiles();
}

void wex::file_history::popup_menu(
  wxWindow*      win,
  int            clear_id,
  const wxPoint& pos) const
{
  auto* menu = new wex::menu();

  for (size_t i = 0; i < size(); i++)
  {
    if (const auto& file(operator[](i)); file.file_exists())
    {
      auto* item = new wxMenuItem(menu, get_base_id() + i, file.filename());

      // We could add a bitmap here, but on MSW shown with black edges.
      // item->SetBitmap(wxTheFileIconsTable->GetSmallImageList()->GetBitmap(
      // wxTheFileIconsTable->GetIconID(file.extension())))
      menu->Append(item);
    }
  }

  if (menu->GetMenuItemCount() > 0)
  {
    menu->append({{}});

    if (clear_id != -1)
    {
      menu->append({{clear_id, wxGetStockLabel(wxID_CLEAR)}});
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

void wex::file_history::use_menu(wxWindowID id, wex::menu* menu)
{
  auto* submenu = new wex::menu;
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
  if (GetCount() > 0 && static_cast<int>(index) < GetMaxFiles())
  {
    bool        error = false;
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
    catch (const std::exception&)
    {
      error = true;
    }

    if (error)
    {
      m_contents.remove(file);
      const_cast<file_history_imp*>(this)->RemoveFileFromHistory(index);
      log::status(_("Removed not existing file")) << file << "from history";
    }
  }

  return wxEmptyString;
}
