////////////////////////////////////////////////////////////////////////////////
// Name:      filedlg.cpp
// Purpose:   Implementation of wxWidgets file dialog class
// Author:    Anton van Wezenbeek
// Created:   2009-10-07
// RCS-ID:    $Id$
// Copyright: (c) 2009 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/extension/filedlg.h>
#include <wx/extension/app.h>
#include <wx/extension/file.h>

wxExFileDialog::wxExFileDialog(
  wxWindow *parent,
  wxExFile* file,
  const wxString &message, 
  const wxString &wildcard,
  long style, 
  const wxPoint &pos, 
  const wxSize &size, 
  const wxString &name)
  : wxFileDialog(
      parent, 
      message, 
      file->GetFileName().GetPath(), 
      file->GetFileName().GetFullName(), 
      wildcard, 
      style, 
      pos, 
      size, 
      name)
  , m_File(file)
  , m_OriginalWildcard(wildcard)
{
}

bool wxExFileDialog::Continue()
{
  if (m_File->GetContentsChanged())
  {
    if (!m_File->GetFileName().FileExists())
    {
      switch (wxMessageBox(
        _("Save changes") + "?",
        _("Confirm"),
        wxYES_NO | wxCANCEL | wxICON_QUESTION))
      {
        case wxYES: 
          // This should be a save dialog.
          wxASSERT(GetWindowStyle() & wxFD_SAVE);

          if (ShowModal(false) != wxID_OK) return false; 
          break;

        case wxNO:     
          m_File->ResetContentsChanged(); 
          break;

        case wxCANCEL: return false; break;
      }
    }
    else
    {
      switch (wxMessageBox(
        _("Save changes to") + ": " + m_File->GetFileName().GetFullPath() + "?",
        _("Confirm"),
        wxYES_NO | wxCANCEL | wxICON_QUESTION))
      {
        case wxYES:    
          m_File->FileSave(); 
          break;

        case wxNO:     
          m_File->ResetContentsChanged(); 
          break;

        case wxCANCEL: 
          return false; 
          break;
      }
    }
  }

  return true;
}

int wxExFileDialog::ShowModal(bool ask_for_continue)
{
  if (ask_for_continue)
  {
    if (!Continue())
    {
      return wxID_CANCEL;
    }
  }

  // First set actual filename etc. according to file.
  SetFilename(m_File->GetFileName().GetFullPath());
  SetDirectory(m_File->GetFileName().GetPath());

  // Override wildcard only if it is default.
  if (m_OriginalWildcard == wxFileSelectorDefaultWildcardStr)
  {
    SetWildcard(wxExApp::GetLexers()->BuildWildCards(m_File->GetFileName()));
  }

  const int result = wxFileDialog::ShowModal();

  if (result == wxID_OK)
  {
    if (GetWindowStyle() & wxFD_SAVE)
    {
      m_File->FileSave(GetPath());
    }
    else
    {
      m_File->FileLoad(wxExFileName(GetPath()));
    }
  }

  return result;
}
