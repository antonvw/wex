////////////////////////////////////////////////////////////////////////////////
// Name:      header.h
// Purpose:   Declaration of wxExHeader class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXHEADER_H
#define _EXHEADER_H

#include <wx/window.h>

class wxExFileName;

/// This class offers methods to make a file header.
class WXDLLIMPEXP_BASE wxExHeader
{
public:
  /// Default constructor.
  /// Sets fields if purpose is not empty.
  wxExHeader(
    const wxString& purpose = wxEmptyString, 
    const wxString& author = wxEmptyString,
    const wxString& email = wxEmptyString,
    const wxString& license = wxEmptyString);

  /// Returns a file header for specified filename,
  /// using it's lexer and data from config,
  /// among which the file purpose.
  const wxString Get(const wxExFileName* filename) const;
  
  /// Sets title fields (if not empty) in the config.
  void Set(
    const wxString& purpose, 
    const wxString& author = wxEmptyString,
    const wxString& email = wxEmptyString,
    const wxString& license = wxEmptyString);

#if wxUSE_GUI
  /// Shows a dialog for getting the purpose for a header,
  /// some other fields are presented as well (if author is empty
  /// in the config).
  int ShowDialog(
    wxWindow* parent,
    const wxString& title = _("File Purpose")) const;

  /// Shows a dialog for getting the puropse, 
  /// and returns the header if OK.
  const wxString ShowDialog(
    const wxExFileName* filename, 
    wxWindow* parent) const {
    if (ShowDialog(parent) == wxID_OK) return Get(filename);
    else return wxEmptyString;}
#endif    
private:
  const wxString m_TextAuthor;
  const wxString m_TextEmail;
  const wxString m_TextLicense;
  const wxString m_TextPurpose;
};
#endif
