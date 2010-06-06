////////////////////////////////////////////////////////////////////////////////
// Name:      filedlg.cpp
// Purpose:   Implementation of wxExtension file dialog class
// Author:    Anton van Wezenbeek
// Created:   2009-10-07
// RCS-ID:    $Id$
// Copyright: (c) 2009 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
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
      size) 
// when compiling under x11 the name is not used as argument,
// so outcommented it here.      
//      name)
  , m_File(file)
{
  // Override wildcard if it is default and file is initialized.
  if (wildcard == wxFileSelectorDefaultWildcardStr &&
      m_File->GetFileName().IsOk())
  {
    SetWildcard(wxExLexers::Get()->BuildWildCards(m_File->GetFileName()));
  }
}

int wxExFileDialog::ShowModalIfChanged(bool show_modal)
{
  if (m_File->GetContentsChanged())
  {
    if (!m_File->GetFileName().IsOk())
    {
      switch (wxMessageBox(
        _("Save changes") + "?",
        _("Confirm"),
        wxYES_NO | wxCANCEL | wxICON_QUESTION))
      {
        case wxYES: 
          return ShowModal();
          break;

        case wxNO:     
          m_File->ResetContentsChanged(); 
          break;

        case wxCANCEL: return wxID_CANCEL; break;
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
          return wxID_CANCEL;
          break;
      }
    }
  }

  if (show_modal)
  {
    return ShowModal();
  }

  return wxID_OK;
}
