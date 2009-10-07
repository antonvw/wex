////////////////////////////////////////////////////////////////////////////////
// Name:      filedlg.cpp
// Purpose:   Implementation of wxWidgets file dialog class
// Author:    Anton van Wezenbeek
// Created:   2009-10-07
// RCS-ID:    $Id$
// Copyright: (c) 2009 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/extension/filedlg.h>

wxExFileDialog::wxExFileDialog(
  wxWindow *parent, 
  const wxString &message, 
  const wxString &defaultDir, 
  const wxString &defaultFile, 
  const wxString &wildcard, 
  long style, 
  const wxPoint &pos, 
  const wxSize &size, 
  const wxString &name)
  : wxFileDialog(parent, message, defaultDir, defaultFile, wildcard, style, pos, size, name)
  , m_Wildcard(wildcard)
{
}

bool wxExFileDialog::FileSaveAs()
{
  wxASSERT(wxTheApp != NULL);

  wxFileDialog dlg(
    wxTheApp->GetTopWindow(),
    wxFileSelectorPromptStr,
    wxEmptyString,
    wxEmptyString,
    m_Wildcard,
    wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

  if (ShowFileDialog(dlg, false) == wxID_CANCEL)
  {
    return false;
  }

  const wxString filename = dlg.GetPath();

  if (!filename.empty())
  {
    m_FileName.Assign(filename);
    m_FileName.SetLexer();
    return FileSave();
  }

  return false;
}

int wxExFileDialog::ShowModal(bool ask_for_continue)
{
  if (ask_for_continue)
  {
    if (!m_File.Continue())
    {
      return wxID_CANCEL;
    }
  }

  SetFilename(m_File.GetFileName().GetFullPath());
  SetDirectory(m_File.GetFileName().GetPath());
  m_Wildcard = GetWildcard();

  return wxFileDialog::ShowModal();
}
