////////////////////////////////////////////////////////////////////////////////
// Name:      util.h
// Purpose:   Include file for wxExtension report utilities
// Author:    Anton van Wezenbeek
// Copyright: (c) 2012 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EX_REPORT_UTIL_H
#define _EX_REPORT_UTIL_H

#include <wx/aui/auibook.h>
#include <wx/filename.h>
#include <wx/extension/listitem.h>
#include <wx/extension/textfile.h> // for wxExFileStatistics

class wxExFrameWithHistory;

/*! \file */

/// Do something (id) for all pages on the notebook.
/// Returns false if page is not a listview file page.
bool wxExForEach(
  /// the notebook
  wxAuiNotebook* notebook, 
  /// ID_LIST_ALL_CLOSE or ID_LIST_ALL_ITEMS
  int id, 
  /// font to be used if id is ID_LIST_ALL_ITEMS
  const wxFont& font = wxFont());

/// Runs make on specified makefile.
/// Results are placed on the list process output, if it can be activated from frame.
/// Returns true if make process could be executed.
long wxExMake(wxExFrameWithHistory* frame, const wxFileName& makefile);

/// Runs a tool on this item, and returns statistics.
const wxExFileStatistics wxExRun(const wxExListItem& item, const wxExTool& tool);

#endif
