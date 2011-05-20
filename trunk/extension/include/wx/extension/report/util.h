/******************************************************************************\
* File:          util.h
* Purpose:       Include file for wxExtension report utilities
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _EX_REPORT_UTIL_H
#define _EX_REPORT_UTIL_H

#include <wx/aui/auibook.h>
#include <wx/filename.h>

class wxExFrameWithHistory;
class wxExListView;

/*! \file */

/// Finds other filenames from the one specified in the same dir structure.
/// Results are put on the listview.
bool wxExFindOtherFileName(
  const wxFileName& filename,
  wxExListView* listview);

/// Do something (id) for all pages on the notebook.
bool wxExForEach(wxAuiNotebook* notebook, int id, const wxFont& font = wxFont());

/// Runs make on specified makefile.
/// Results are placed on the list process output, if it can be activated from frame.
bool wxExMake(wxExFrameWithHistory* frame, const wxFileName& makefile);
#endif
