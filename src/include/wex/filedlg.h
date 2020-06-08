////////////////////////////////////////////////////////////////////////////////
// Name:      filedlg.h
// Purpose:   Declaration of wex::file_dialog class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/window-data.h>
#include <wx/filedlg.h>

namespace wex
{
  class file;

  /// Adds a file and / or a hexmode checkbox to wxFileDialog.
  class file_dialog : public wxFileDialog
  {
  public:
    /// Default constructor for hexmode only.
    file_dialog(
      /// window data
      const data::window& data = data::window().style(wxFD_DEFAULT_STYLE));

    /// Constructor for file and a hexmode.
    file_dialog(
      /// specify file
      file* file,
      /// window data
      const data::window& data = data::window().style(wxFD_DEFAULT_STYLE));

    /// Virtual interface.
    int ShowModal() override;

    /// Other methods.

    /// Returns true if hexmode checkbox is (was) checked.
    bool hexmode() const { return m_hexmode; };

    /// Shows the dialog depending on the changes on the file.
    /// If you specify show_modal then dialog is always shown.
    int show_modal_if_changed(bool show_modal = false);

  private:
    file* m_file{nullptr};
    bool  m_hexmode{false};
  };
}; // namespace wex
