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

#ifdef LOGGING
void LogKeys(const map < wxString wxWindow* >& keys)
{
  int i = 0;
  for (const auto& it : keys)
  {
    wxLogMessage("key[%d]=%s", i++, it->first());
  }
}
#endif
  
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
  // Here you could use another art provider.
  // SetArtProvider(new wxAuiSimpleTabArt); 
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
    return NULL;
  }
  
  m_Keys[key] = page;
  m_Windows[page] = key;

#ifdef LOGGING
  wxLogMessage("added key: %s", key.c_str());
  LogKeys(m_Keys);
#endif

  return page;
}

bool wxExNotebook::DeletePage(const wxString& key)
{
  const int index = GetPageIndexByKey(key);

  if (index != wxNOT_FOUND && wxAuiNotebook::DeletePage(index))
  {
    wxWindow* page = m_Keys[key];
    m_Keys.erase(key);
    m_Windows.erase(page);
    
    if (m_Frame != NULL && m_Keys.empty())
    {
      m_Frame->SyncCloseAll(GetId());
    }
#ifdef LOGGING
    wxLogMessage("deleted key: %s index: %d", key.c_str(), index);
    LogKeys(m_Keys);
#endif

    return true;
  }
  else
  {
    return false;
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
        
          const wxString key = m_Windows[GetPage(page)];
          m_Windows.erase(GetPage(page));
          m_Keys.erase(key);
          wxAuiNotebook::DeletePage(page);
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

  if (m_Frame != NULL && m_Keys.empty())
  {
    m_Frame->SyncCloseAll(GetId());
  }

  return true;
}

const wxString wxExNotebook::GetKeyByPage(wxWindow* page) const
{
  const auto it = m_Windows.find(page);
  return (it != m_Windows.end() ? it->second: wxString(wxEmptyString));
}

wxWindow* wxExNotebook::GetPageByKey(const wxString& key) const
{
  const auto it = m_Keys.find(key);
  return (it != m_Keys.end() ? it->second: NULL);
}

int wxExNotebook::GetPageIndexByKey(const wxString& key) const
{
  wxWindow* page = GetPageByKey(key);
  
  if (page != NULL)
  {
    return GetPageIndex(page);
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
  if (!wxAuiNotebook::InsertPage(page_idx, page, (text.empty() ? key: text), select, bitmap))
  {
    return NULL;
  }

  m_Keys[key] = page;
  m_Windows[page] = key;

#ifdef LOGGING
  wxLogMessage("inserted key: %s index: %d", key.c_str(), page_idx);
  LogKeys(m_Keys);
#endif

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
    const int sel = event.GetSelection();
    
    if (sel != wxNOT_FOUND)
    {
      if (m_Frame != NULL && !m_Frame->AllowClose(GetId(), GetPage(sel)))
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
        
        if (m_Frame != NULL)
        {
          if (m_Keys.empty()) m_Frame->SyncCloseAll(GetId());
          m_Frame->HideExBar();
        }
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
  const int index = GetPageIndexByKey(key);

  if (index == wxNOT_FOUND)
  {
    return NULL;
  }

  wxAuiNotebook::SetSelection(index);
  return GetPage(index);
}
  
#endif // wxUSE_GUI
