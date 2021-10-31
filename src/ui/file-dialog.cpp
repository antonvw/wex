////////////////////////////////////////////////////////////////////////////////
// Name:      file-dialog.cpp
// Purpose:   Implementation of wex::file_dialog class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/core.h>
#include <wex/core/file.h>
#include <wex/factory/lexers.h>
#include <wex/ui/file-dialog.h>
#include <wx/checkbox.h>
#include <wx/checklst.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

class extra_panel : public wxPanel
{
public:
  explicit extra_panel(wxWindow* parent);
  bool checked() const { return m_checked; }

private:
  const int   m_id_checkbox;
  bool        m_checked{false};
  wxCheckBox* m_cb;
};

extra_panel::extra_panel(wxWindow* parent)
  : wxPanel(parent)
  , m_id_checkbox(NewControlId())
  , m_cb(new wxCheckBox(this, m_id_checkbox, "Hex"))
{
  Bind(
    wxEVT_CHECKBOX,
    [&](wxCommandEvent& event)
    {
      m_checked = !m_checked;
    },
    m_id_checkbox);

  auto* sizerTop = new wxBoxSizer(wxHORIZONTAL);
  sizerTop->Add(
    new wxStaticText(this, wxID_ANY, "Mode:"),
    wxSizerFlags().Centre().Border());
  sizerTop->AddSpacer(10);
  sizerTop->Add(m_cb, wxSizerFlags().Centre().Border());
  sizerTop->AddSpacer(5);

  SetSizerAndFit(sizerTop);
}

static wxWindow* create_extra_panel(wxWindow* parent)
{
  return new extra_panel(parent);
}

wex::file_dialog::file_dialog(const data::window& data)
  : wxFileDialog(
      data.parent(),
      data.title(),
      std::string(),
      std::string(),
      data.wildcard(),
      data.style(),
      data.pos(),
      data.size())
{
}

wex::file_dialog::file_dialog(wex::file* file, const data::window& data)
  : wxFileDialog(
      data.parent(),
      data.title(),
      file->path().parent_path(),
      file->path().filename(),
      data.wildcard(),
      data.style(),
      data.pos(),
      data.size(),
      data.name())
  , m_file(file)
{
  if (
    data.wildcard() == wxFileSelectorDefaultWildcardStr &&
    m_file->path().stat().is_ok())
  {
    std::string wildcards = _("All Files") + " (" +
                            wxFileSelectorDefaultWildcardStr + ") |" +
                            wxFileSelectorDefaultWildcardStr;
    const auto& allow_ext(data.allow_move_path_extension());

    for (const auto& it : lexers::get()->get_lexers())
    {
      if (!it.extensions().empty())
      {
        const std::string wildcard =
          it.display_lexer() + " (" + it.extensions() + ") |" + it.extensions();
        wildcards =
          (allow_ext == file->path().extension() &&
               matches_one_of(file->path().filename(), it.extensions()) ?
             wildcard + "|" + wildcards :
             wildcards + "|" + wildcard);
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

  if (m_file->is_contents_changed())
  {
    if (!m_file->path().stat().is_ok())
    {
      switch (wxMessageBox(
        _("Save changes") + "?",
        _("Confirm"),
        wxYES_NO | wxCANCEL | wxICON_QUESTION))
      {
        case wxYES:
          return ShowModal();
        case wxNO:
          reset = true;
          break;
        case wxCANCEL:
          return wxID_CANCEL;
      }
    }
    else
    {
      switch (wxMessageBox(
        _("Save changes to") + ": " + m_file->path().string() + "?",
        _("Confirm"),
        wxYES_NO | wxCANCEL | wxICON_QUESTION))
      {
        case wxYES:
          m_file->file_save();
          break;
        case wxNO:
          reset = true;
          break;
        case wxCANCEL:
          return wxID_CANCEL;
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
  if (!(GetWindowStyle() & wxFD_SAVE) && (GetWindowStyle() & wxFD_HEX_MODE))
  {
    SetExtraControlCreator(&create_extra_panel);
  }

  const auto id = wxFileDialog::ShowModal();

  if (auto* extra = GetExtraControl(); id == wxID_OK && extra != nullptr)
  {
    m_hexmode = static_cast<extra_panel*>(extra)->checked();
  }

  return id;
}
