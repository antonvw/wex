////////////////////////////////////////////////////////////////////////////////
// Name:      filedlg.h
// Purpose:   Declaration of wxExtension file dialog class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/filedlg.h>
#include <wx/extension/window-data.h>

class wxExFile;

/// Adds an wxExFile to wxFileDialog.
class WXDLLIMPEXP_BASE wxExFileDialog : public wxFileDialog
{
public:
  /// Constructor.
  wxExFileDialog(
    /// specify file
    wxExFile* file,
    /// window data
    const wxExWindowData& data = wxExWindowData().Style(wxFD_DEFAULT_STYLE),
    /// wildcard
    /// if wildcard is default and file is initialized, 
    /// the wildcard is taken from the file
    const wxString& wildcard = wxFileSelectorDefaultWildcardStr);

  /// Shows the dialog depending on the changes on the file.
  /// If you specify show_modal then dialog is always shown.
  int ShowModalIfChanged(bool show_modal = false);
private:
  wxExFile* m_File;
};
