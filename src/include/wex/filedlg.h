////////////////////////////////////////////////////////////////////////////////
// Name:      filedlg.h
// Purpose:   Declaration of wex::file_dialog class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/filedlg.h>
#include <wex/window-data.h>

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
      const window_data& data = window_data().style(wxFD_DEFAULT_STYLE),
      /// wildcard
      /// if wildcard is default and file is initialized, 
      /// the wildcard is taken from the file
      const std::string& wildcard = wxFileSelectorDefaultWildcardStr);

    /// Constructor for file and a hexmode.
    file_dialog(
      /// specify file
      file* file,
      /// window data
      const window_data& data = window_data().style(wxFD_DEFAULT_STYLE),
      /// wildcard
      /// if wildcard is default and file is initialized, 
      /// the wildcard is taken from the file
      const std::string& wildcard = wxFileSelectorDefaultWildcardStr);

    /// Virtual interface.
    virtual int ShowModal() override;
    
    /// Other methods.

    /// Returns true if hexmode checkbox is (was) checked.
    bool hexmode() const {return m_hexmode;};

    /// Shows the dialog depending on the changes on the file.
    /// If you specify show_modal then dialog is always shown.
    int show_modal_if_changed(bool show_modal = false);
  private:
    file* m_file {nullptr};
    bool m_hexmode {false};
  };
};
