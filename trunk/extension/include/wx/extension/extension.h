/******************************************************************************\
* File:          extension.h
* Purpose:       Include file for most wxWidgets extension classes
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _WXEXTENSION_H
#define _WXEXTENSION_H

#include <wx/extension/version.h>
#include <wx/extension/base.h>
#include <wx/extension/app.h>
#include <wx/extension/file.h>
#include <wx/extension/util.h>

class wxExConfig;

/// Returns a header.
const wxString wxExHeader(
  const wxExFileName& filename,
  wxExConfig* config);

/// Shows a dialog for getting the purpose for a header.
int wxExHeaderDialog(wxWindow* parent, wxExConfig* config);

#if wxUSE_GUI
/// Calls OpenFile for wxExFrame, if this is your top window.
void wxExOpenFile(const wxFileName& filename, long open_flags = 0);
#endif // wxUSE_GUI
#endif
