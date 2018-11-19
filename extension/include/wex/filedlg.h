////////////////////////////////////////////////////////////////////////////////
// Name:      filedlg.h
// Purpose:   Declaration of wex::file_dialog class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/filedlg.h>
#include <wex/window-data.h>

namespace wex
{
  class file;

  /// Adds an file to wxFileDialog.
  class file_dialog : public wxFileDialog
  {
  public:
    /// Constructor.
    file_dialog(
      /// specify file
      file* file,
      /// window data
      const window_data& data = window_data().style(wxFD_DEFAULT_STYLE),
      /// wildcard
      /// if wildcard is default and file is initialized, 
      /// the wildcard is taken from the file
      const std::string& wildcard = wxFileSelectorDefaultWildcardStr);

    /// Shows the dialog depending on the changes on the file.
    /// If you specify show_modal then dialog is always shown.
    int show_modal_if_changed(bool show_modal = false);
  private:
    file* m_File;
  };
};
