////////////////////////////////////////////////////////////////////////////////
// Name:      filedlg.cpp
// Purpose:   Implementation of wxExtension file dialog class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/filedlg.h>
#include <wx/extension/file.h>
#include <wx/extension/lexers.h>
#include <wx/extension/util.h>

wxExFileDialog::wxExFileDialog(
  wxWindow *parent,
  wxExFile* file,
  const wxString &message, 
  const wxString &wildcard,
  const wxExWindowData& data)
  : wxFileDialog(
      parent, 
      message, 
      file->GetFileName().GetPath(), 
      file->GetFileName().GetFullName(), 
      wildcard, 
      data.Style(), 
      data.Pos(), 
      data.Size()) 
// TODO: when compiling under x11 the name is not used as argument,
// so outcommented it here.      
//      name)
  , m_File(file)
{
  // Override wildcard if it is default and file is initialized.
  if (wildcard == wxFileSelectorDefaultWildcardStr &&
      m_File->GetFileName().GetStat().IsOk())
  {
    wxString wildcards = 
      _("All Files") + wxString::Format(" (%s)|%s",
        wxFileSelectorDefaultWildcardStr,
        wxFileSelectorDefaultWildcardStr);

    for (const auto& it : wxExLexers::Get()->GetLexers())
    {
      if (!it.GetExtensions().empty())
      {
        const wxString wildcard =
          it.GetDisplayLexer() +
          " (" + it.GetExtensions() + ") |" +
          it.GetExtensions();
        wildcards = (wxExMatchesOneOf(file->GetFileName().GetFullName(), it.GetExtensions()) ?
          wildcard + "|" + wildcards: wildcards + "|" + wildcard);
      }
    }

    SetWildcard(wildcards);
  }
}

int wxExFileDialog::ShowModalIfChanged(bool show_modal)
{
  bool reset = false;
  
  if (m_File->GetContentsChanged())
  {
    if (!m_File->GetFileName().GetStat().IsOk())
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
