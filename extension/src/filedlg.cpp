////////////////////////////////////////////////////////////////////////////////
// Name:      filedlg.cpp
// Purpose:   Implementation of wxExtension file dialog class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2014 Anton van Wezenbeek
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
// TODO: when compiling under x11 the name is not used as argument,
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
  bool reset = false;
  
  if (m_File->GetContentsChanged())
  {
    if (!m_File->GetFileName().IsOk())
    {
      switch (wxMessageBox(
        _("Save changes") + "?",
        _("Confirm"),
        wxYES_NO | wxCANCEL | wxICON_QUESTION))
      {
        case wxYES: return ShowModal(); break;
        case wxNO: reset = true; break;
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
        case wxYES: m_File->FileSave(); break;
        case wxNO: reset = true; break;
        case wxCANCEL: return wxID_CANCEL; break;
      }
    }
  }

  if (show_modal)
  {
    const int result = ShowModal();
    
    if (reset && result != wxID_CANCEL)
    {
      m_File->ResetContentsChanged(); 
    }
  
    return result;
  }

  return wxID_OK;
}
