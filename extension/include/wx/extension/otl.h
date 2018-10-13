////////////////////////////////////////////////////////////////////////////////
// Name:      otl.h
// Purpose:   Declaration of wex::otl class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#if wexUSE_OTL
#define OTL_ODBC
#define OTL_STL
#define OTL_DESTRUCTORS_DO_NOT_THROW
//#define OTL_UNICODE
#if __UNIX__
#define OTL_ODBC_UNIX
#endif
#include <otlv4.h>

#include <wx/extension/version.h>
#include <wx/extension/window-data.h>

class wxGrid;
class wxStyledTextCtrl;

namespace wex
{
  /// Offers methods to the otl connection.
  class otl
  {
  public:
    /// Default constructor.
    /// Initializes the otl connection using specified threaded mode.
    otl(int threaded_mode = 0);

    /// Destructor.
    /// Logs off.
   ~otl();

    /// Returns the datasource connected to or to connect to.
    const std::string Datasource() const;

    /// Returns true if we are connected.
    bool IsConnected() const {return m_Connect.connected > 0;};

    /// Logs off.
    /// Returns true if you were connected.
    bool Logoff();

    /// Logons to the datasource (shows a connection dialog if parent 
    /// is not nullptr).
    /// max_items specifies max number of datasources in the combobox and config.
    /// Returns false if dialog cancelled or logon fails.
    bool Logon(const window_data& data = window_data());

    /// Runs the query using direct_exec and returns result.
    long Query(const std::string& query);

    /// Runs the query and puts results on the grid.
    /// If empty_results then the grid is cleared first.
    /// Returns number of rows appended.
    long Query(const std::string& query,
      wxGrid* grid,
      bool& stopped,
      bool empty_results = true,
      int buffer_size = 1024);

    /// Runs the query and appends results to the stc.
    /// Returns number of lines added.
    long Query(const std::string& query,
      wxStyledTextCtrl* stc,
      bool& stopped,
      int buffer_size = 1024);

    /// Returns the OTL version.
    static const version_info VersionInfo();
  private:
    otl_connect m_Connect;
  };
};
#endif
