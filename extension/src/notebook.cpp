////////////////////////////////////////////////////////////////////////////////
// Name:      notebook.cpp
// Purpose:   Implementation of class wxExNotebook
// Author:    Anton van Wezenbeek
// Copyright: (c) 2013 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/extension/notebook.h>
#include <wx/extension/defs.h>
#include <wx/extension/filedlg.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/stc.h>

#if wxUSE_GUI

#undef LOGGING

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
  // Here you could use another atr provider.
  //SetArtProvider(new wxAuiSimpleTabArt); 
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
    wxFAIL;
    return NULL;
  }

  const wxString use_text = (text.empty() ? key: text);

  if (!wxAuiNotebook::AddPage(page, use_text, select, bitmap))
  {
    wxFAIL;
    return NULL;
  }
  
  m_MapPages[key] = page;

#ifdef LOGGING
  wxLogMessage("added page key: %s text: %s page: %d",
    key.c_str(), text.c_str(), page->GetId());
  LogMapPages();
#endif

  return page;
}

bool wxExNotebook::DeletePage(const wxString& key)
{
  wxWindow* page = GetPageByKey(key);

  wxASSERT(page != NULL);

  if (wxAuiNotebook::DeletePage(GetPageIndex(page)))
  {
#ifdef LOGGING
    wxLogMessage("deleted page: %s page: %d",
      key.c_str(), page->GetId());
    LogMapPages();
#endif

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

#ifdef LOGGING
  wxLogMessage("erased page key: %s",
    key.c_str());
  LogMapPages();
#endif

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
  int page = 0;
    
  // The page should be an int (no), otherwise page >= 0 never fails!
  while (page < GetPageCount() )
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
      page++;
      break;
      
    case ID_ALL_STC_SAVE:
      if (stc->GetFile().GetContentsChanged())
      {
        stc->GetFile().FileSave();
      }
      page++;
      break;

    case ID_ALL_STC_SET_LEXER: 
      // At this moment same as themed change,
      // as we want default colour updates as well.
      stc->SetLexer(stc->GetLexer().GetDisplayLexer());
      page++;
      break;

    case ID_ALL_STC_SET_LEXER_THEME: 
      stc->SetLexer(stc->GetLexer().GetDisplayLexer());
      page++;
      break;

    case ID_ALL_STC_SYNC: 
      stc->Sync(wxConfigBase::Get()->ReadBool("AllowSync", true)); 
      page++;
      break;
      
    default: 
      wxFAIL; 
      page++;
      break;
    }
  }

  return true;
}

const wxString wxExNotebook::GetKeyByPage(wxWindow* page) const
{
#ifdef LOGGING
  wxLogMessage("get key by page: %d",
    page->GetId());
  LogMapPages();
#endif

  for (
    auto it = m_MapPages.begin();
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
  const auto it = m_MapPages.find(key);

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

#ifdef LOGGING
  wxLogMessage("inserted page key: %s text: %s page: %d",
    key.c_str(), text.c_str(), page->GetId());
  LogMapPages();
#endif

  return page;
}

void wxExNotebook::LogMapPages() const
{
  for (
    std::map<wxString, wxWindow*>::const_iterator it = m_MapPages.begin();
    it != m_MapPages.end();
    ++it)
  {
     wxLogMessage("map[%s]=%d", it->first.c_str(), it->second->GetId());
  }
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
      const int sel = event.GetSelection();
      
      if (sel == wxNOT_FOUND)
      {
        wxFAIL;
        return;
      }
      
      if (!m_Frame->AllowClose(GetId(), GetPage(sel)))
      {
        event.Veto();
      }
      else
      {
        const wxString key = GetKeyByPage(GetPage(sel));
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

wxWindow* wxExNotebook::SetSelection(const wxString& key)
{
  wxWindow* page = GetPageByKey(key);

  if (page != NULL)
  {
    wxAuiNotebook::SetSelection(GetPageIndex(page));
  }
  
  return page;
}
  
#endif // wxUSE_GUI
