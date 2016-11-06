////////////////////////////////////////////////////////////////////////////////
// Name:      notebook.cpp
// Purpose:   Implementation of class wxExNotebook
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/notebook.h>

#if wxUSE_GUI

wxExNotebook::wxExNotebook(wxWindow* parent,
  wxExManagedFrame* frame,
  wxWindowID id,
  const wxPoint& pos,
  const wxSize& size,
  long style)
  : wxAuiNotebook(parent, id, pos, size, style)
  , m_Frame(frame)
{
  // Here you could use another art provider.
  // SetArtProvider(new wxAuiSimpleTabArt); 

  switch (id)
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
    const int sel = event.GetSelection();
    if (sel != wxNOT_FOUND)
    {
      if (m_Frame != nullptr && !m_Frame->AllowClose(GetId(), GetPage(sel)))
      {
        event.Veto();
      }
      else
      {
        wxWindow* page = GetPage(sel);
        const wxString key = m_Windows[page];
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
  const wxString& key,
  const wxString& text,
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

const wxString wxExNotebook::ChangeSelection(const wxString& key)
{
  const auto index = GetPageIndexByKey(key);
  int previous;
  
  if (index != wxNOT_FOUND && 
    ((previous = wxAuiNotebook::ChangeSelection(index))) >= 0)
  {
    wxWindow* page = m_Keys[key];
    m_Keys[key] = page;
    m_Windows[page] = key;
    return GetKeyByPage(GetPage(previous));
  }
  
  return wxEmptyString;
}
  
bool wxExNotebook::DeletePage(const wxString& key)
{
  const auto index = GetPageIndexByKey(key);

  if (index != wxNOT_FOUND && wxAuiNotebook::DeletePage(index))
  {
    wxWindow* page = m_Keys[key];
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

const wxString wxExNotebook::GetCurrentPage()
{
  wxWindow* page = wxAuiNotebook::GetCurrentPage();
  return GetKeyByPage(page);
}
  
wxWindow* wxExNotebook::InsertPage(
  size_t page_idx,
  wxWindow* page,
  const wxString& key,
  const wxString& text,
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
  const wxString& key,
  const wxString& new_key,
  const wxString& text,
  const wxBitmap& bitmap)
{
  const auto index = GetPageIndexByKey(key);

  if (index == wxNOT_FOUND || !wxAuiNotebook::SetPageText(index, text))
  {
    return false;
  }

  wxWindow* page = m_Keys[key];
  m_Keys.erase(key);
  m_Keys[new_key] = page;
  m_Windows[page] = new_key;
  
  if (bitmap.IsOk())
  {
    SetPageBitmap(index, bitmap);
  }

  return true;
}

wxWindow* wxExNotebook::SetSelection(const wxString& key)
{
  const auto index = GetPageIndexByKey(key);

  if (index == wxNOT_FOUND)
  {
    return nullptr;
  }

  wxAuiNotebook::SetSelection(index);
  
  wxWindow* page = GetPage(index);
  
  page->SetFocus();
  
  return page;
}
  
bool wxExNotebook::Split(const wxString& key, int direction)
{
  const auto index = GetPageIndexByKey(key);

  if (index == wxNOT_FOUND)
  {
    return false;
  }
  
  wxAuiNotebook::Split(index, direction);
  
  return true;
}
#endif // wxUSE_GUI
