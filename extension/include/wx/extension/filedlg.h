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
    wxWindow *parent,
    wxExFile* file,
    const wxString &message=wxFileSelectorPromptStr, 
    const wxString &wildcard=wxFileSelectorDefaultWildcardStr,
    const wxExWindowData& data = wxExWindowData().Style(wxFD_DEFAULT_STYLE));

  /// Shows the dialog depending on the changes on the file.
  /// If you specify show_modal then dialog is always shown.
  int ShowModalIfChanged(bool show_modal = false);
private:
  wxExFile* m_File;
};
