////////////////////////////////////////////////////////////////////////////////
// Name:      notebook.cpp
// Purpose:   Implementation of class wex::notebook
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/notebook.h>
#include <wex/item-vector.h>
#include <wex/itemdlg.h>

namespace wex
{
  const std::vector < item > notebook_config_items() 
  {
    return std::vector < item > ({
      {_("tab.Font"), 
        item::FONTPICKERCTRL, 
        wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT)},
      {_("tab.Art provider"), 
        item::COMBOBOX, 
        std::list < std::string >{"simple", "default"},
        control_data().window(window_data().style(wxCB_READONLY))}});
  };
}
    
wex::notebook::notebook(const window_data& data)
  : wxAuiNotebook(
      data.parent(), 
      data.id(), 
      data.pos(), 
      data.size(), 
      data.style() == DATA_NUMBER_NOT_SET ?
        wxAUI_NB_DEFAULT_STYLE:
        data.style())
  , m_frame(dynamic_cast<managed_frame*>(wxTheApp->GetTopWindow()))
{
  config_get();

  Bind(wxEVT_AUINOTEBOOK_PAGE_CHANGED, [=](wxAuiNotebookEvent& event) {
    event.Skip(); // call base
    if (m_frame != nullptr)
    {
      m_frame->on_notebook(GetId(), GetPage(event.GetSelection()));
    }});
  
  Bind(wxEVT_AUINOTEBOOK_PAGE_CLOSE, [=](wxAuiNotebookEvent& event) {
    if (const auto sel = event.GetSelection(); sel != wxNOT_FOUND)
    {
      if (m_frame != nullptr && !m_frame->allow_close(GetId(), GetPage(sel)))
      {
        event.Veto();
      }
      else
      {
        auto* page = GetPage(sel);
        const auto key = m_windows[page];
        m_windows.erase(page);
        m_keys.erase(key);
        event.Skip(); // call base
        
        if (m_frame != nullptr)
        {
          if (m_keys.empty()) m_frame->sync_close_all(GetId());
          m_frame->hide_ex_bar();
        }
      }
    }});
}

wxWindow* wex::notebook::add_page(
  wxWindow* page,
  const std::string& key,
  const std::string& caption,
  bool select,
  const wxBitmap& bitmap)
{
  if (!AddPage(page, (caption.empty() ? key: caption), select, bitmap))
  {
    return nullptr;
  }
  
  m_keys[key] = page;
  m_windows[page] = key;

  return page;
}

const std::string wex::notebook::change_selection(const std::string& key)
{
  int previous;
  
  if (const auto index = page_index_by_key(key); 
    index != wxNOT_FOUND && ((previous = wxAuiNotebook::ChangeSelection(index))) >= 0)
  {
    auto* page = m_keys[key];
    m_keys[key] = page;
    m_windows[page] = key;
    return key_by_page(GetPage(previous));
  }
  
  return std::string();
}
 
int wex::notebook::config_dialog(const window_data& par)
{
  const window_data data(window_data(par).title(_("Tab Options")));

  if (m_config_dialog == nullptr)
  {
    m_config_dialog = new item_dialog(notebook_config_items(), data);
  }

  return (data.button() & wxAPPLY) ?
    m_config_dialog->Show(): m_config_dialog->ShowModal();
}

void wex::notebook::config_get()
{
  const auto& ci(notebook_config_items());
  const item_vector& iv(&ci);

  if (const auto& p(
    iv.find<std::list<std::string>>(_("tab.Art provider")));
    p.empty())
  {
    log("no art provider");
  }
  else if (p.front() == "simple")
  {
    SetArtProvider(new wxAuiSimpleTabArt); 
  }
  else if (p.front() == "default")
  {
    SetArtProvider(new wxAuiDefaultTabArt); 
  }
  else
  {
    log("not supported art provider") << p.front();
  }

  SetFont(iv.find<wxFont>(_("tab.Font")));
}

bool wex::notebook::delete_page(const std::string& key)
{
  if (const auto index = page_index_by_key(key);
    index != wxNOT_FOUND && DeletePage(index))
  {
    auto* page = m_keys[key];
    m_keys.erase(key);
    m_windows.erase(page);

    if (m_frame != nullptr && m_keys.empty())
    {
      m_frame->sync_close_all(GetId());
    }

    return true;
  }
  else
  {
    return false;
  }
}

const std::string wex::notebook::current_page_key()
{
  auto* page = GetCurrentPage();
  return key_by_page(page);
}
  
wxWindow* wex::notebook::insert_page(
  size_t page_idx,
  wxWindow* page,
  const std::string& key,
  const std::string& caption,
  bool select,
  const wxBitmap& bitmap)
{
  if (!InsertPage(page_idx, page, (caption.empty() ? key: caption), select, bitmap))
  {
    return nullptr;
  }

  m_keys[key] = page;
  m_windows[page] = key;

  return page;
}

void wex::notebook::rearrange(int direction)
{
  for (size_t i = 0; i < GetPageCount(); ++i)
  {
    wxAuiNotebook::Split(i, direction);
  }
}

bool wex::notebook::set_page_text(
  const std::string& key,
  const std::string& new_key,
  const std::string& caption,
  const wxBitmap& bitmap)
{
  if (const auto index = page_index_by_key(key);
    index == wxNOT_FOUND || !SetPageText(index, caption))
  {
    return false;
  }
  else 
  {
    auto* page = m_keys[key];
    m_keys.erase(key);
    m_keys[new_key] = page;
    m_windows[page] = new_key;
    
    if (bitmap.IsOk())
    {
      SetPageBitmap(index, bitmap);
    }

    return true;
  }
}

wxWindow* wex::notebook::set_selection(const std::string& key)
{
  if (const auto index = page_index_by_key(key); index == wxNOT_FOUND)
  {
    return nullptr;
  }
  else 
  {
    wxAuiNotebook::SetSelection(index);
    auto* page = GetPage(index);
    page->SetFocus();
    return page;
  }
}
  
bool wex::notebook::split(const std::string& key, int direction)
{
  if (const auto index = page_index_by_key(key); index == wxNOT_FOUND)
  {
    return false;
  }
  else 
  {
    wxAuiNotebook::Split(index, direction);
    return true;
  }
}
