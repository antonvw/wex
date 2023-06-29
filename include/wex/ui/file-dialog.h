////////////////////////////////////////////////////////////////////////////////
// Name:      file-dialog.h
// Purpose:   Declaration of wex::file_dialog class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2009-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/factory/window.h>
#include <wx/filedlg.h>

/// Extra flag to show hex mode button on dialog.
#define wxFD_HEX_MODE wxFD_SHOW_HIDDEN << 2

namespace wex
{
class file;

/// Adds a file and / or a hexmode checkbox to wxFileDialog.
/// The hexmode option can be set by setting wxFD_HEX_MODE
/// in the window::data.
class file_dialog : public wxFileDialog
{
public:
  /// Constructor.
  file_dialog(
    /// specify file (might be nullptr)
    file* file,
    /// window data
    const data::window& data = data::window().style(wxFD_DEFAULT_STYLE));

  /// Virtual interface.
  int ShowModal() override;

  /// Other methods.

  /// Returns true if hexmode checkbox is (was) checked.
  bool is_hexmode() const { return m_hexmode; }

  /// Shows the dialog depending on the changes on the file.
  /// If you specify show_modal then dialog is always shown.
  int show_modal_if_changed(bool show_modal = false);

private:
  file* m_file{nullptr};
  bool  m_hexmode{false};
};
}; // namespace wex
