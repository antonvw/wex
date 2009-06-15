/******************************************************************************\
* File:          notebook.cpp
* Purpose:       Implementation of class wxExNotebook
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/extension/notebook.h>
#include <wx/extension/base.h> // for wxExManagedFrame
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

  if (m_MapPages.empty() && m_Frame != NULL)
  {
    m_Frame->SyncCloseAll(GetId());
  }
}

bool wxExNotebook::ForEach(int id)
{
  // The page should be an int, otherwise page >= 0 never fails!
  for (int page = GetPageCount() - 1; page >= 0; page--)
  {
    // When trying to cast to wxExFile, there is an error:
    // e:\lib\wxExtension\v2.0\src\notebook.cpp(96): error C2440: 'static_cast' : cannot convert from 'wxWindow *' to 'const wxExFile *'
    // e:\lib\wxExtension\v2.0\src\notebook.cpp(96): error C2039: 'ms_classInfo' : is not a member of 'wxExFile'
    // E:\lib\wxExtension\v2.0\include\wx\extension\file.h(95) : see declaration of 'wxExFile'

    // Try to get an wxExSTC out of the page.
    wxExSTC* stc = wxDynamicCast(GetPage(page), wxExSTC);

    if (stc == NULL)
    {
      wxLogError("Notebook page: %d (%s) cannot be cast to an wxExSTC",
        page,
        GetPageText(page).c_str());

      // Do not return false, otherwise close all would not finish.
      continue;
    }

    switch (id)
    {
    case ID_ALL_STC_COLOURISE: stc->Colourise(); break;
    case ID_ALL_STC_CONFIG_GET: stc->ConfigGet(); break;
    case ID_ALL_STC_SET_LEXER: stc->SetLexer(); break;

    case ID_ALL_STC_CLOSE:
      if (!stc->Continue()) return false;
      DeletePage(GetKeyByPage(GetPage(page)));
      break;

    case ID_ALL_STC_SAVE:
      if (stc->GetContentsChanged())
      {
        stc->FileSave();
      }
      break;

    default: wxFAIL; break;
    }
  }

  return true;
}

const wxString wxExNotebook::GetKeyByPage(wxWindow* page) const
{
  for (
    std::map<wxString, wxWindow*>::const_iterator it = m_MapPages.begin();
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

const wxString wxExNotebook::GetKeys() const
{
  wxString keys;

  for (
    std::map<wxString, wxWindow*>::const_iterator it = m_MapPages.begin();
    it != m_MapPages.end();
    it++)
  {
    keys += it->first + "\n";
  }

  return keys;
}

wxWindow* wxExNotebook::GetPageByKey(const wxString& key) const
{
  std::map<wxString,wxWindow*>::const_iterator it = m_MapPages.find(key);

  if (it != m_MapPages.end())
  {
    return it->second;
  }
  else
  {
    return NULL;
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
      if (!m_Frame->AllowClose(GetId(), GetPage(GetSelection())))
      {
        event.Veto();
      }
      else
      {
        const wxString key = GetKeyByPage(GetPage(GetSelection()));
        ErasePage(key);
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
  const wxString& text)
{
  wxWindow* page = GetPageByKey(key);

  if (page != NULL)
  {
    m_MapPages.erase(key);
    m_MapPages[new_key] = page;
    wxAuiNotebook::SetPageText(GetPageIndex(page), text);
  }

  return (page != NULL);
}

#endif // wxUSE_GUI
