/******************************************************************************\
* File:          notebook.cpp
* Purpose:       Implementation of class exNotebook
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2008 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/extension/notebook.h>
#include <wx/extension/stc.h>

#if wxUSE_GUI

BEGIN_EVENT_TABLE(exNotebook, wxAuiNotebook)
  EVT_AUINOTEBOOK_PAGE_CHANGED(wxID_ANY, exNotebook::OnNotebook)
  EVT_AUINOTEBOOK_PAGE_CLOSE(wxID_ANY, exNotebook::OnNotebook)
END_EVENT_TABLE()

exNotebook::exNotebook(wxWindow* parent,
  exManagedFrame* frame,
  wxWindowID id,
  const wxPoint& pos,
  const wxSize& size,
  long style)
  : wxAuiNotebook(parent, id, pos, size, style)
  , m_Frame(frame)
{
}

wxWindow* exNotebook::AddPage(
    wxWindow* page,
    const wxString& key,
    const wxString& text,
    bool select,
    const wxBitmap& bitmap)
{
  if (GetPageByKey(key, select) != NULL)
  {
    wxLogError("Page with key: %s already exists", key.c_str());
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

bool exNotebook::DeletePage(const wxString& key)
{
  wxWindow* page = GetPageByKey(key);

  wxASSERT(page != NULL);

  m_MapPages.erase(key);

  return wxAuiNotebook::DeletePage(GetPageIndex(page));
}

bool exNotebook::ErasePage(size_t n)
{
  for (
    std::map<wxString,wxWindow*>::iterator it = m_MapPages.begin();
    it != m_MapPages.end();
    ++it)
  {
    if (it->second == GetPage(n))
    {
      m_MapPages.erase(it);
      return true;
    }
  }

  wxLogError("Could not find page: %d to delete", n);

  return false;
}

bool exNotebook::ForEach(int id)
{
  if (GetPageCount() == 0)
  {
    // Nothing to do.
    return true;
  }

  // The page should be an int, otherwise page >= 0 never fails!
  for (int page = GetPageCount() - 1; page >= 0; page--)
  {
    // When trying to cast to exFile, there is an error:
    // e:\lib\wxExtension\v2.0\src\notebook.cpp(96): error C2440: 'static_cast' : cannot convert from 'wxWindow *' to 'const exFile *'
    // e:\lib\wxExtension\v2.0\src\notebook.cpp(96): error C2039: 'ms_classInfo' : is not a member of 'exFile'
    // E:\lib\wxExtension\v2.0\include\wx\extension\file.h(95) : see declaration of 'exFile'

    // Try to get an exSTC out of the page.
    exSTC* stc = wxDynamicCast(GetPage(page), exSTC);

    if (stc == NULL)
    {
      wxLogError("Notebook page: %d (%s) cannot be cast to an exSTC",
        page,
        GetPageText(page).c_str());

      // Do not return false, otherwise close all would not finish.
      continue;
    }

    switch (id)
    {
    case ID_ALL_STC_COLOURISE: stc->Colourise(); break;
    case ID_ALL_STC_CONFIG_GET: stc->ConfigGet(); break;
    case ID_ALL_STC_PRINT: stc->Print(false); break;
    case ID_ALL_STC_SET_LEXER: stc->SetLexer(); break;

    case ID_ALL_STC_CLOSE:
      if (!stc->Continue()) return false;

      ErasePage(page);

      if (m_MapPages.empty() && m_Frame != NULL)
      {
        m_Frame->SyncCloseAll(GetId());
      }

      if (!wxAuiNotebook::DeletePage(page)) return false;
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

const wxString exNotebook::GetKeyByPage(wxWindow* page) const
{
  if (!m_MapPages.empty())
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
  }

  return wxEmptyString;
}

const wxString exNotebook::GetKeys() const
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

wxWindow* exNotebook::GetPageByKey(const wxString& key, bool select)
{
  if (m_MapPages.empty()) return NULL;

  std::map<wxString,wxWindow*>::const_iterator it = m_MapPages.find(key);

  if (it == m_MapPages.end()) return NULL;

  if (select)
    SetSelection(GetPageIndex(it->second));

  return it->second;
}

void exNotebook::OnNotebook(wxAuiNotebookEvent& event)
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
        ErasePage(GetSelection());

        if (m_MapPages.empty() && m_Frame != NULL)
        {
          m_Frame->SyncCloseAll(GetId());
        }

        event.Skip(); // call base
      }
    }
  }
  else
  {
    wxFAIL;
  }
}

bool exNotebook::SetPageText(
  const wxString& key,
  const wxString& text)
{
  wxWindow* page = GetPageByKey(key);

  if (page != NULL)
  {
    wxAuiNotebook::SetPageText(GetPageIndex(page), text);
  }

  return (page != NULL);
}

bool exNotebook::SetPageText(
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
