////////////////////////////////////////////////////////////////////////////////
// Name:      notebook.cpp
// Purpose:   Implementation of class wxExNotebook
// Author:    Anton van Wezenbeek
// Copyright: (c) 2014 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/wupdlock.h>
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
    return NULL;
  }

  const wxString use_text = (text.empty() ? key: text);

  if (!wxAuiNotebook::AddPage(page, use_text, select, bitmap))
  {
    wxFAIL;
    return NULL;
  }
  
  m_Keys.push_back(key);

#ifdef LOGGING
  wxLogMessage("added key: %s text: %s page: %d",
    key.c_str(), text.c_str(), page->GetId());
  LogKeys();
#endif

  return page;
}

bool wxExNotebook::DeletePage(const wxString& key)
{
  const int index = GetPageIndexByKey(key);

  if (index != wxNOT_FOUND && wxAuiNotebook::DeletePage(index))
  {
    EraseKey(index);
#ifdef LOGGING
    wxLogMessage("deleted key: %s page: %d",
      key.c_str(), index);
    LogKeys();
#endif

    return true;
  }
  else
  {
    return false;
  }
}

void wxExNotebook::EraseKey(int page)
{
  auto it = m_Keys.begin();
  m_Keys.erase(it + page);

#ifdef LOGGING
  wxLogMessage("erased key: %d", page);
  LogKeys();
#endif

  if (m_Frame != NULL && m_Keys.empty())
  {
    m_Frame->SyncCloseAll(GetId());
  }
}

bool wxExNotebook::ForEach(int id)
{
  wxWindowUpdateLocker locker(m_Frame != NULL ? (wxWindow*)m_Frame: (wxWindow*)this);
  
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
    case ID_ALL_STC_CLOSE_OTHERS:
      {
        const int sel = GetSelection();

        if ((id == ID_ALL_STC_CLOSE_OTHERS && sel != page) ||
             id == ID_ALL_STC_CLOSE)
        {
          wxExFileDialog dlg(this, &stc->GetFile());
          
          if (dlg.ShowModalIfChanged() == wxID_CANCEL) 
          {
            return false;
          }
        
          wxAuiNotebook::DeletePage(page);
          EraseKey(page);
        }
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
      stc->SetLexer(stc->GetLexer().GetDisplayLexer());
      break;

    case ID_ALL_STC_SET_LEXER_THEME: 
      stc->SetLexer(stc->GetLexer().GetDisplayLexer());
      break;

    case ID_ALL_STC_SYNC: 
      stc->Sync(wxConfigBase::Get()->ReadBool("AllowSync", true)); 
      break;
      
    default: 
      wxFAIL; 
      break;
    }
  }

  return true;
}

const wxString wxExNotebook::GetKeyByPage(wxWindow* page) const
{
#ifdef LOGGING
  wxLogMessage("get key by page: %d", page->GetId());
  LogKeys();
#endif

  const int index = GetPageIndex(page);
  
  if (index != wxNOT_FOUND)
  {
    return m_Keys[index];
  }

  wxFAIL;

  return wxEmptyString;
}

wxWindow* wxExNotebook::GetPageByKey(const wxString& key) const
{
  const int index = GetPageIndexByKey(key);
  
  if (index != wxNOT_FOUND)
  {
    return GetPage(index);
  }
  
  return NULL;
}

int wxExNotebook::GetPageIndexByKey(const wxString& key) const
{
  // std::find gives compile error
  // const auto it = std::find(m_Keys.begin(), m_Keys.end(), key);
  int i = 0;
  for (const auto& it : m_Keys)
  {
    if (it == key)
    {
      return i;
    }
  
    i++;
  }

  return wxNOT_FOUND;
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
    return NULL;
  }

  const wxString use_text = (text.empty() ? key: text);

  if (!wxAuiNotebook::InsertPage(page_idx, page, use_text, select, bitmap))
  {
    wxFAIL;
    return NULL;
  }

  auto it = m_Keys.begin();
  m_Keys.insert(it + page_idx, key);

#ifdef LOGGING
  wxLogMessage("inserted key: %s text: %s page: %d",
    key.c_str(), text.c_str(), page_idx);
  LogKeys();
#endif

  return page;
}

void wxExNotebook::LogKeys() const
{
  int i = 0;
  for (const auto& it : m_Keys)
  {
    wxLogMessage("key[%d]=%s", i++, it);
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
        EraseKey(sel);
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
  const int index = GetPageIndexByKey(key);

  if (index != wxNOT_FOUND)
  {
    if (!wxAuiNotebook::SetPageText(index, text))
    {
      wxFAIL;
    }
    
    m_Keys[index] = new_key;
    
    if (bitmap.IsOk())
    {
      SetPageBitmap(index, bitmap);
    }
  }

  return (index != wxNOT_FOUND);
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
