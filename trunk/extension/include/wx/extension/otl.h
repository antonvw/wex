/******************************************************************************\
* File:          otl.h
* Purpose:       Declaration of wxExOTL class
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 2008-2009, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _EXOTL_H
#define _EXOTL_H

#if USE_OTL
#define OTL_ODBC
#define OTL_STL
//#define OTL_UNICODE
#if __WXGTK__
#define OTL_ODBC_UNIX
#endif
#include <otlv4.h>

class wxExConfig;
class wxGrid;
class wxStyledTextCtrl;

/// Offers methods to the otl database.
class wxExOTL
{
public:
  /// Default constructor.
  /// Initializes the otl connection using threaded mode.
  wxExOTL(const int  threaded_mode = 0);

  /// Destructor.
  /// Logs off.
 ~wxExOTL();

  /// Returns member.
  otl_connect& GetConnect() {return m_Connect;};

  /// Returns connected member from the connection.
  bool IsConnected() const {return m_Connect.connected > 0;};

  /// Logons to the database (shows a database dialog).
  /// max_items specifies max number of datasources in the combobox and config.
  /// Returns false if dialog cancelled or logon fails.
  bool Logon(wxExConfig* config, int max_items = 4);

  /// Run the query and return results.
  long Query(const wxString& query);

#if wxUSE_GRID
  /// Run the query and put results on the grid (if the grid is shown).
  /// If empty_results then the grid is cleared first.
  long Query(const wxString& query,
    wxGrid* grid,
    bool& stopped,
    bool empty_results = true);
#endif // wxUSE_GRID

  /// Run the query and append results to the stc.
  long Query(const wxString& query,
    wxStyledTextCtrl* stc,
    bool& stopped);

  /// Returns the OTL version as a string.
  static const wxString Version();
private:
  otl_connect m_Connect;
};

#endif // USE_OTL
#endif
