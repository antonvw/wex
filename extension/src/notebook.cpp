////////////////////////////////////////////////////////////////////////////////
// Name:      notebook.cpp
// Purpose:   Implementation of class wxExNotebook
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/notebook.h>

#if wxUSE_GUI

wxExNotebook::wxExNotebook(const wxExWindowData& data)
  : wxAuiNotebook(
      data.Parent(), 
      data.Id(), 
      data.Pos(), 
      data.Size(), 
      data.Style() == DATA_NUMBER_NOT_SET ?
        wxAUI_NB_TOP | wxAUI_NB_TAB_SPLIT | wxAUI_NB_TAB_MOVE | wxAUI_NB_SCROLL_BUTTONS: 
        data.Style())
  , m_Frame(dynamic_cast<wxExManagedFrame*>(wxTheApp->GetTopWindow()))
{
  // Here you could use another art provider.
  // SetArtProvider(new wxAuiSimpleTabArt); 

  switch (data.Id())
  {
    case ID_NOTEBOOK_EDITORS:
      SetFont(wxConfigBase::Get()->ReadObject(_("Tab font"), 
        wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT))); break;
    case ID_NOTEBOOK_LISTS:
      SetFont(wxConfigBase::Get()->ReadObject(_("List tab font"), 
        wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT))); break;
    case ID_NOTEBOOK_PROJECTS:
      SetFont(wxConfigBase::Get()->ReadObject(_("Project tab font"), 
        wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT))); break;
  }
  
  Bind(wxEVT_AUINOTEBOOK_PAGE_CHANGED, [=](wxAuiNotebookEvent& event) {
    event.Skip(); // call base
    if (m_Frame != nullptr)
    {
      m_Frame->OnNotebook(GetId(), GetPage(event.GetSelection()));
    }});
  
  Bind(wxEVT_AUINOTEBOOK_PAGE_CLOSE, [=](wxAuiNotebookEvent& event) {
    if (const auto sel = event.GetSelection(); sel != wxNOT_FOUND)
    {
      if (m_Frame != nullptr && !m_Frame->AllowClose(GetId(), GetPage(sel)))
      {
        event.Veto();
      }
      else
      {
        auto* page = GetPage(sel);
        const auto key = m_Windows[page];
        m_Windows.erase(page);
        m_Keys.erase(key);
        event.Skip(); // call base
        
        if (m_Frame != nullptr)
        {
          if (m_Keys.empty()) m_Frame->SyncCloseAll(GetId());
          m_Frame->HideExBar();
        }
      }
    }});
}

wxWindow* wxExNotebook::AddPage(
  wxWindow* page,
  const std::string& key,
  const std::string& text,
  bool select,
  const wxBitmap& bitmap)
{
  if (!wxAuiNotebook::AddPage(page, (text.empty() ? key: text), select, bitmap))
  {
    return nullptr;
  }
  
  m_Keys[key] = page;
  m_Windows[page] = key;

  return page;
}

const std::string wxExNotebook::ChangeSelection(const std::string& key)
{
  int previous;
  
  if (const auto index = GetPageIndexByKey(key); 
    index != wxNOT_FOUND && ((previous = wxAuiNotebook::ChangeSelection(index))) >= 0)
  {
    auto* page = m_Keys[key];
    m_Keys[key] = page;
    m_Windows[page] = key;
    return GetKeyByPage(GetPage(previous));
  }
  
  return std::string();
}
  
bool wxExNotebook::DeletePage(const std::string& key)
{
  if (const auto index = GetPageIndexByKey(key);
    index != wxNOT_FOUND && wxAuiNotebook::DeletePage(index))
  {
    auto* page = m_Keys[key];
    m_Keys.erase(key);
    m_Windows.erase(page);
    page = nullptr;
    
    if (m_Frame != nullptr && m_Keys.empty())
    {
      m_Frame->SyncCloseAll(GetId());
    }

    return true;
  }
  else
  {
    return false;
  }
}

const std::string wxExNotebook::GetCurrentPage()
{
  auto* page = wxAuiNotebook::GetCurrentPage();
  return GetKeyByPage(page);
}
  
wxWindow* wxExNotebook::InsertPage(
  size_t page_idx,
  wxWindow* page,
  const std::string& key,
  const std::string& text,
  bool select,
  const wxBitmap& bitmap)
{
  if (!wxAuiNotebook::InsertPage(page_idx, page, (text.empty() ? key: text), select, bitmap))
  {
    return nullptr;
  }

  m_Keys[key] = page;
  m_Windows[page] = key;

  return page;
}

void wxExNotebook::Rearrange(int direction)
{
  for (size_t i = 0; i < GetPageCount(); ++i)
  {
    wxAuiNotebook::Split(i, direction);
  }
}

bool wxExNotebook::SetPageText(
  const std::string& key,
  const std::string& new_key,
  const std::string& text,
  const wxBitmap& bitmap)
{
  if (const auto index = GetPageIndexByKey(key);
    index == wxNOT_FOUND || !wxAuiNotebook::SetPageText(index, text))
  {
    return false;
  }
  else 
  {
    auto* page = m_Keys[key];
    m_Keys.erase(key);
    m_Keys[new_key] = page;
    m_Windows[page] = new_key;
    
    if (bitmap.IsOk())
    {
      SetPageBitmap(index, bitmap);
    }

    return true;
  }
}

wxWindow* wxExNotebook::SetSelection(const std::string& key)
{
  if (const auto index = GetPageIndexByKey(key); index == wxNOT_FOUND)
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
  
bool wxExNotebook::Split(const std::string& key, int direction)
{
  if (const auto index = GetPageIndexByKey(key); index == wxNOT_FOUND)
  {
    return false;
  }
  else 
  {
    wxAuiNotebook::Split(index, direction);
    return true;
  }
}
#endif // wxUSE_GUI
