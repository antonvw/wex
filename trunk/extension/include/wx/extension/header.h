/******************************************************************************\
* File:          header.h
* Purpose:       Declaration of wxExHeader class
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 2009, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _EXHEADER_H
#define _EXHEADER_H

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

class wxExFileName;

/// This class offers methods to make a file header.
class wxExHeader
{
public:
  /// Default constructor.
  wxExHeader();

  /// Returns a file header for specified filename,
  /// using it's lexer and data from config,
  /// among which the file purpose.
  const wxString Get(const wxExFileName* filename) const;

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
private:
  const wxString m_TextAuthor;
  const wxString m_TextEmail;
  const wxString m_TextLicense;
  const wxString m_TextPurpose;
};
#endif
