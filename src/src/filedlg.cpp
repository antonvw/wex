////////////////////////////////////////////////////////////////////////////////
// Name:      filedlg.cpp
// Purpose:   Implementation of wex::file_dialog class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/checklst.h>
#include <wex/filedlg.h>
#include <wex/file.h>
#include <wex/lexers.h>
#include <wex/util.h>

class extra_panel : public wxPanel
{
public:
  extra_panel(wxWindow *parent);
  bool checked() const {return m_checked;};
private:
  bool m_checked {false};
  wxCheckBox *m_cb;
};

const auto id_checkbox = wxWindow::NewControlId(); 

extra_panel::extra_panel(wxWindow *parent)
  : wxPanel(parent)
  , m_cb(new wxCheckBox(this, id_checkbox, "Hex"))
{
  Bind(wxEVT_CHECKBOX, [&](wxCommandEvent& event) {
    m_checked = !m_checked;}, id_checkbox);

  auto *sizerTop = new wxBoxSizer(wxHORIZONTAL);
  sizerTop->Add(new wxStaticText(this, wxID_ANY, "Mode:"),
    wxSizerFlags().Centre().Border());
  sizerTop->AddSpacer(10);
  sizerTop->Add(m_cb, wxSizerFlags().Centre().Border());
  sizerTop->AddSpacer(5);

  SetSizerAndFit(sizerTop);
}

static wxWindow* create_extra_panel(wxWindow *parent)
{
  return new extra_panel(parent);
}

wex::file_dialog::file_dialog(
  const window_data& data, const std::string& wildcard)
  : wxFileDialog(
      data.parent(), 
      data.title(), 
      std::string(), 
      std::string(), 
      wildcard, 
      data.style(), 
      data.pos(), 
      data.size()) 
{
  SetWildcard(wildcard);
}
      
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
  , m_file(file)
{
  if (wildcard == wxFileSelectorDefaultWildcardStr &&
      m_file->get_filename().stat().is_ok())
  {
    std::string wildcards = 
      _("All Files") + " (" + wxFileSelectorDefaultWildcardStr + ") |" +  
        wxFileSelectorDefaultWildcardStr;

    for (const auto& it : lexers::get()->get_lexers())
    {
      if (!it.extensions().empty())
      {
        const std::string wildcard =
          it.display_lexer() +
          " (" + it.extensions() + ") |" +
          it.extensions();
        wildcards = (matches_one_of(
          file->get_filename().fullname(), it.extensions()) ?
          wildcard + "|" + wildcards: wildcards + "|" + wildcard);
      }
    }

    SetWildcard(wildcards);
  }
}

int wex::file_dialog::show_modal_if_changed(bool show_modal)
{
  if (m_file == nullptr)
  {
    return wxID_CANCEL;
  }

  bool reset = false;
  
  if (m_file->get_contents_changed())
  {
    if (!m_file->get_filename().stat().is_ok())
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
        _("Save changes to") + ": " + m_file->get_filename().string() + "?",
        _("Confirm"),
        wxYES_NO | wxCANCEL | wxICON_QUESTION))
      {
        case wxYES: m_file->file_save(); break;
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
      m_file->reset_contents_changed(); 
    }
  
    return result;
  }

  return wxID_OK;
}

int wex::file_dialog::ShowModal()
{
  SetExtraControlCreator(&create_extra_panel);

  const auto id = wxFileDialog::ShowModal(); 
  
  if (id == wxID_OK)
  {
    auto * extra = GetExtraControl();
    m_hexmode = static_cast<extra_panel*>(extra)->checked();
  }

  return id;
}
