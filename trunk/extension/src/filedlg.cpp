////////////////////////////////////////////////////////////////////////////////
// Name:      filedlg.cpp
// Purpose:   Implementation of wxExtension file dialog class
// Author:    Anton van Wezenbeek
// Created:   2009-10-07
// RCS-ID:    $Id$
// Copyright: (c) 2009 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/extension/filedlg.h>
#include <wx/extension/file.h>
#include <wx/extension/lexers.h>

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

int wxExFileDialog::ShowModal(bool ask_for_continue)
{
  if (ask_for_continue)
  {
    return ShowModalIfChanged();
  }

  // First set actual filename etc. according to file.
  SetFilename(m_File->GetFileName().GetFullPath());
  SetDirectory(m_File->GetFileName().GetPath());

  // Override wildcard only if it is default.
  if (m_OriginalWildcard == wxFileSelectorDefaultWildcardStr)
  {
    SetWildcard(wxExLexers::Get()->BuildWildCards(m_File->GetFileName()));
  }

  return wxFileDialog::ShowModal();
}

int wxExFileDialog::ShowModalIfChanged()
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
