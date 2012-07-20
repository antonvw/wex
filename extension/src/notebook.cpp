////////////////////////////////////////////////////////////////////////////////
// Name:      notebook.cpp
// Purpose:   Implementation of class wxExNotebook
// Author:    Anton van Wezenbeek
// Copyright: (c) 2012 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/notebook.h>
#include <wx/extension/defs.h>
#include <wx/extension/filedlg.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/stc.h>

#if wxUSE_GUI

BEGIN_EVENT_TABLE(wxExNotebook, wxAuiNotebook)
  EVT_AUINOTEBOOK_PAGE_CHANGED(wxID_ANY, wxExNotebook::OnNotebook)
  EVT_AUINOTEBOOK_PAGE_CLOSE(wxID_ANY, wxExNotebook::OnNotebook)
END_EVENT_TABLE()

wxExNotebook::wxExNotebook(wxWindow* parent,
  wxExManagedFrame* frame,
  wxWindowID id,
  const wxPoint& pos,
  const wxSize& size,
  long style)
  : wxAuiNotebook(parent, id, pos, size, style)
  , m_Frame(frame)
{
}

wxWindow* wxExNotebook::AddPage(
  wxWindow* page,
  const wxString& key,
  const wxString& text,
  bool select,
  const wxBitmap& bitmap)
{
  if (GetPageByKey(key) != NULL)
  {
    return NULL;
  }

  const wxString use_text = (text.empty() ? key: text);

  if (!wxAuiNotebook::AddPage(page, use_text, select, bitmap))
  {
    wxFAIL;
    return NULL;
  }

  m_MapPages[key] = page;

  return page;
}

bool wxExNotebook::DeletePage(const wxString& key)
{
  wxWindow* page = GetPageByKey(key);

  wxASSERT(page != NULL);

  if (wxAuiNotebook::DeletePage(GetPageIndex(page)))
  {
    ErasePage(key);
    return true;
  }
  else
  {
    wxFAIL;
    return false;
  }
}

void wxExNotebook::ErasePage(const wxString& key)
{
  m_MapPages.erase(key);

  if (m_Frame != NULL)
  {
    if (m_MapPages.empty())
    {
      m_Frame->SyncCloseAll(GetId());
    }
  }
}

bool wxExNotebook::ForEach(int id)
{
  // The page should be an int (no), otherwise page >= 0 never fails!
  for (int page = GetPageCount() - 1; page >= 0; page--)
  {
    // When trying to cast to wxExFile, there is an error:
    // src\notebook.cpp(96): error C2440: 'static_cast' : cannot convert from 'wxWindow *' to 'const wxExFile *'
    // src\notebook.cpp(96): error C2039: 'ms_classInfo' : is not a member of 'wxExFile'
    // include\wx\extension\file.h(95) : see declaration of 'wxExFile'

    // Try to get an wxExSTC out of the page.
    wxExSTC* stc = wxDynamicCast(GetPage(page), wxExSTC);

    if (stc == NULL)
    {
      wxFAIL;

      // Do not return false, otherwise close all would not finish.
      continue;
    }

    switch (id)
    {
    case ID_ALL_STC_CLOSE:
      {
      wxExFileDialog dlg(this, &stc->GetFile());
      if (dlg.ShowModalIfChanged() == wxID_CANCEL) return false;
      DeletePage(GetKeyByPage(GetPage(page)));
      }
      break;

    case ID_ALL_STC_CONFIG_GET: 
      stc->ConfigGet(); 
      break;
      
    case ID_ALL_STC_SAVE:
      if (stc->GetFile().GetContentsChanged())
      {
        stc->GetFile().FileSave();
      }
      break;

    case ID_ALL_STC_SET_LEXER: 
      // At this moment same as themed change,
      // as we want default colour updates as well.
      stc->SetLexer(stc->GetLexer().GetDisplayLexer(), true);
      break;

    case ID_ALL_STC_SET_LEXER_THEME: 
      stc->SetLexer(stc->GetLexer().GetDisplayLexer(), true);
      break;

    default: wxFAIL; break;
    }
  }

  return true;
}

const wxString wxExNotebook::GetKeyByPage(wxWindow* page) const
{
  for (
#ifdef wxExUSE_CPP0X	
    auto it = m_MapPages.begin();
#else
    std::map<wxString, wxWindow*>::const_iterator it = m_MapPages.begin();
#endif	
    it != m_MapPages.end();
    ++it)
  {
    if (it->second == page)
    {
      return it->first;
    }
  }

  wxFAIL;

  return wxEmptyString;
}

wxWindow* wxExNotebook::GetPageByKey(const wxString& key) const
{
#ifdef wxExUSE_CPP0X	
  const auto it = m_MapPages.find(key);
#else
  const std::map<wxString, wxWindow*>::const_iterator it = m_MapPages.find(key);
#endif  

  if (it != m_MapPages.end())
  {
    return it->second;
  }
  else
  {
    return NULL;
  }
}

int wxExNotebook::GetPageIndexByKey(const wxString& key) const
{
  wxWindow* page = GetPageByKey(key);
  
  if (page != NULL)
  {
    return GetPageIndex(page);
  }
  
  return -1;
}

wxWindow* wxExNotebook::InsertPage(
  size_t page_idx,
  wxWindow* page,
  const wxString& key,
  const wxString& text,
  bool select,
  const wxBitmap& bitmap)
{
  if (GetPageByKey(key) != NULL)
  {
    wxFAIL;
    return NULL;
  }

  const wxString use_text = (text.empty() ? key: text);

  if (!wxAuiNotebook::InsertPage(page_idx, page, use_text, select, bitmap))
  {
    wxFAIL;
    return NULL;
  }

  m_MapPages[key] = page;

  return page;
}

void wxExNotebook::OnNotebook(wxAuiNotebookEvent& event)
{
  if (event.GetEventType() == wxEVT_COMMAND_AUINOTEBOOK_PAGE_CHANGED)
  {
    event.Skip(); // call base

    if (m_Frame != NULL)
    {
      m_Frame->OnNotebook(GetId(), GetPage(event.GetSelection()));
    }
  }
  else if (event.GetEventType() == wxEVT_COMMAND_AUINOTEBOOK_PAGE_CLOSE)
  {
    if (m_Frame != NULL)
    {
      if (!m_Frame->AllowClose(GetId(), GetPage(GetSelection())))
      {
        event.Veto();
      }
      else
      {
        const wxString key = GetKeyByPage(GetPage(GetSelection()));
        ErasePage(key);
        m_Frame->HideExBar();
        event.Skip(); // call base
      }
    }
  }
  else
  {
    wxFAIL;
  }
}

wxWindow* wxExNotebook::SelectPage(const wxString& key)
{
  wxWindow* page = GetPageByKey(key);

  if (page != NULL)
  {
    SetSelection(GetPageIndex(page));
  }
  
  return page;
}
  
bool wxExNotebook::SetPageText(
  const wxString& key,
  const wxString& new_key,
  const wxString& text,
  const wxBitmap& bitmap)
{
  wxWindow* page = GetPageByKey(key);

  if (page != NULL)
  {
    m_MapPages.erase(key);
    m_MapPages[new_key] = page;

    if (!wxAuiNotebook::SetPageText(GetPageIndex(page), text))
    {
      wxFAIL;
    }
    
    if (bitmap.IsOk())
    {
      SetPageBitmap(GetPageIndex(page), bitmap);
    }
  }

  return (page != NULL);
}

#endif // wxUSE_GUI
