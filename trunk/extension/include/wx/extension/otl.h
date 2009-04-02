/******************************************************************************\
* File:          otl.h
* Purpose:       Declaration of otl related things
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 2008, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _EXOTL_H
#define _EXOTL_H

#include <wx/grid.h>
#include <wx/stc/stc.h>

#if USE_OTL
#define OTL_ODBC
#define OTL_STL
//#define OTL_UNICODE
#if __WXGTK__
#define OTL_ODBC_UNIX
#endif
#include <otlv4.h>

class exConfig;

/*! \file */
/// Shows a database dialog, and if db is specified also tries to login.
/// max_items specifies max number of datasources in the combobox and config.
bool exOTLDialog(
  exConfig* config,
  otl_connect* db = NULL,
  int max_items = 4);

/// Returns the OTL version as a string.
const wxString exOTLVersion();

#if wxUSE_GRID
/// Run the query using db and put results on the grid (if the grid is shown).
/// If empty_results then the grid is cleared first.
long exOTLQueryToGrid(
  otl_connect* db,
  const wxString& query,
  wxGrid* grid,
  bool& stopped,
  bool empty_results = true);
#endif // wxUSE_GRID

/// Run the query using db and append results to the stc.
long exOTLQueryToSTC(
  otl_connect* db,
  const wxString& query,
  wxStyledTextCtrl* stc,
  bool& stopped);

#endif // USE_OTL

#endif
