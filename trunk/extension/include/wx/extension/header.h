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

class wxExConfig;
class wxExFileName;

/// This class offers methods to make a file header.
class wxExHeader
{
public:
  /// Constructor.
  wxExHeader(wxExConfig* config);

  /// Returns the header.
  const wxString Get(const wxExFileName* filename) const;

  /// Shows a dialog for getting the purpose for a header.
  int ShowDialog(wxWindow* parent) const;

  /// Shows a dialog, and returns the header if OK.
  const wxString ShowDialog(const wxExFileName* filename, wxWindow* parent) const {
    if (ShowDialog(parent) == wxID_OK) return Get(filename);
    else return wxEmptyString;}
private:
  wxExConfig* m_Config;
};
#endif
