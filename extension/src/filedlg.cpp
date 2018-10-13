////////////////////////////////////////////////////////////////////////////////
// Name:      filedlg.cpp
// Purpose:   Implementation of wex::file_dialog class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/filedlg.h>
#include <wx/extension/file.h>
#include <wx/extension/lexers.h>
#include <wx/extension/util.h>

wex::file_dialog::file_dialog(
  wex::file* file,
  const window_data& data,
  const std::string& wildcard)
  : wxFileDialog(
      data.Parent(), 
      data.Title(), 
      file->GetFileName().GetPath(), 
      file->GetFileName().GetFullName(), 
      wildcard, 
      data.Style(), 
      data.Pos(), 
      data.Size()) 
// when compiling under x11 the name is not used as argument,
// so outcommented it here.      
//      name)
  , m_File(file)
{
  if (wildcard == wxFileSelectorDefaultWildcardStr &&
      m_File->GetFileName().GetStat().IsOk())
  {
    std::string wildcards = 
      _("All Files") + wxString::Format(" (%s)|%s",
        wxFileSelectorDefaultWildcardStr,
        wxFileSelectorDefaultWildcardStr);

    for (const auto& it : lexers::Get()->GetLexers())
    {
      if (!it.GetExtensions().empty())
      {
        const std::string wildcard =
          it.GetDisplayLexer() +
          " (" + it.GetExtensions() + ") |" +
          it.GetExtensions();
        wildcards = (matches_one_of(file->GetFileName().GetFullName(), it.GetExtensions()) ?
          wildcard + "|" + wildcards: wildcards + "|" + wildcard);
      }
    }

    SetWildcard(wildcards);
  }
}

int wex::file_dialog::ShowModalIfChanged(bool show_modal)
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
        case wxYES: return ShowModal();
        case wxNO: reset = true; break;
        case wxCANCEL: return wxID_CANCEL;
      }
    }
    else
    {
      switch (wxMessageBox(
        _("Save changes to") + ": " + m_File->GetFileName().Path().string() + "?",
        _("Confirm"),
        wxYES_NO | wxCANCEL | wxICON_QUESTION))
      {
        case wxYES: m_File->FileSave(); break;
        case wxNO: reset = true; break;
        case wxCANCEL: return wxID_CANCEL;
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
