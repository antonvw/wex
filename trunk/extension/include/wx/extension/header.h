/******************************************************************************\
* File:          header.h
* Purpose:       Include file for wxextension utility functions
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
#include <wx/filename.h>

class wxExConfig;
class wxExFileName;

/// Returns a header.
const wxString wxExHeader(
  const wxExFileName* filename,
  wxExConfig* config);

/// Shows a dialog for getting the purpose for a header.
int wxExHeaderDialog(wxWindow* parent, wxExConfig* config);
#endif
