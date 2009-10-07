////////////////////////////////////////////////////////////////////////////////
// Name:      filedlg.h
// Purpose:   Declaration of wxWidgets file dialog class
// Author:    Anton van Wezenbeek
// Created:   2009-10-07
// RCS-ID:    $Id$
// Copyright: (c) 2009 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXFILEDLG_H
#define _EXFILEDLG_H

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/filedlg.h>
#include <wx/extension/file.h>

/// Adds an wxExFile, and a wildcard member that.
class wxExFileDialog : public wxFileDialog
{
public:
  /// Constructor.
  wxExFileDialog(
    wxWindow *parent, 
    const wxString &message=wxFileSelectorPromptStr, 
    const wxString &defaultDir=wxEmptyString, 
    const wxString &defaultFile=wxEmptyString, 
    const wxString &wildcard=wxFileSelectorDefaultWildcardStr, 
    long style=wxFD_DEFAULT_STYLE, 
    const wxPoint &pos=wxDefaultPosition, 
    const wxSize &size=wxDefaultSize, 
    const wxString &name=wxFileDialogNameStr);

  /// Shows file dialog and calls FileSave.
  bool FileSaveAs();

  /// Sets the wild card member.
  void SetWildcard(const wxString& wildcard) {m_Wildcard = wildcard;};

  /// Invoked ShowModal on dialog, and returns dialog return code.
  int ShowModal(bool ask_for_continue = true);
private:
  wxExFile m_File; ///< the file
  wxString m_Wildcard;
};
#endif
