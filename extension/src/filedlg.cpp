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
#include <wex/filedlg.h>
#include <wex/file.h>
#include <wex/lexers.h>
#include <wex/util.h>

wex::file_dialog::file_dialog(
  wex::file* file,
  const window_data& data,
  const std::string& wildcard)
  : wxFileDialog(
      data.parent(), 
      data.title(), 
      file->get_filename().get_path(), 
      file->get_filename().fullname(), 
      wildcard, 
      data.style(), 
      data.pos(), 
      data.size()) 
// when compiling under x11 the name is not used as argument,
// so outcommented it here.      
//      name)
  , m_File(file)
{
  if (wildcard == wxFileSelectorDefaultWildcardStr &&
      m_File->get_filename().stat().is_ok())
  {
    std::string wildcards = 
      _("All Files") + wxString::Format(" (%s)|%s",
        wxFileSelectorDefaultWildcardStr,
        wxFileSelectorDefaultWildcardStr);

    for (const auto& it : lexers::get()->get_lexers())
    {
      if (!it.extensions().empty())
      {
        const std::string wildcard =
          it.display_lexer() +
          " (" + it.extensions() + ") |" +
          it.extensions();
        wildcards = (matches_one_of(file->get_filename().fullname(), it.extensions()) ?
          wildcard + "|" + wildcards: wildcards + "|" + wildcard);
      }
    }

    SetWildcard(wildcards);
  }
}

int wex::file_dialog::show_modal_if_changed(bool show_modal)
{
  bool reset = false;
  
  if (m_File->get_contents_changed())
  {
    if (!m_File->get_filename().stat().is_ok())
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
        _("Save changes to") + ": " + m_File->get_filename().data().string() + "?",
        _("Confirm"),
        wxYES_NO | wxCANCEL | wxICON_QUESTION))
      {
        case wxYES: m_File->file_save(); break;
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
      m_File->reset_contents_changed(); 
    }
  
    return result;
  }

  return wxID_OK;
}
