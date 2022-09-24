////////////////////////////////////////////////////////////////////////////////
// Name:      file-dialog.cpp
// Purpose:   Implementation of wex::file_dialog class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/core.h>
#include <wex/core/file.h>
#include <wex/syntax/lexers.h>
#include <wex/ui/file-dialog.h>
#include <wx/filedlgcustomize.h>
#include <wx/msgdlg.h>

class file_dialog_hook : public wxFileDialogCustomizeHook
{
public:
  void AddCustomControls(wxFileDialogCustomize& customizer) override
  {
    m_checkbox = customizer.AddCheckBox("Hex");
  }

  void TransferDataFromCustomControls() override
  {
    m_hexmode = m_checkbox->GetValue();
  }

  auto is_hexmode() const { return m_hexmode; }

private:
  wxFileDialogCheckBox* m_checkbox{nullptr};
  bool                  m_hexmode{false};
};

wex::file_dialog::file_dialog(wex::file* file, const data::window& data)
  : wxFileDialog(
      data.parent(),
      data.title(),
      file != nullptr ? file->path().parent_path() : std::string(),
      file != nullptr ? file->path().filename() : std::string(),
      data.wildcard(),
      data.style(),
      data.pos(),
      data.size(),
      data.name())
  , m_file(file)
{
  if (
    m_file != nullptr && data.wildcard() == wxFileSelectorDefaultWildcardStr &&
    m_file->path().stat().is_ok())
  {
    std::string wildcards(
      _("All Files") + " (" + wxFileSelectorDefaultWildcardStr + ") |" +
      wxFileSelectorDefaultWildcardStr);
    const auto& allow_ext(data.allow_move_path_extension());

    for (const auto& it : lexers::get()->get_lexers())
    {
      if (!it.extensions().empty())
      {
        const std::string wildcard(
          it.display_lexer() + " (" + it.extensions() + ") |" +
          it.extensions());
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
    const auto result(ShowModal());

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
  file_dialog_hook hook;

  if (!(GetWindowStyle() & wxFD_SAVE) && (GetWindowStyle() & wxFD_HEX_MODE))
  {
    SetCustomizeHook(hook);
  }

  const auto id(wxFileDialog::ShowModal());

  m_hexmode = hook.is_hexmode();

  return id;
}
