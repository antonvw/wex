////////////////////////////////////////////////////////////////////////////////
// Name:      odbc.h
// Purpose:   Declaration of wex::odbc class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#if wexUSE_ODBC

#include <wex/core/version.h>
#include <wex/factory/window.h>

#include <memory>

class wxGrid;
class wxStyledTextCtrl;

namespace wex
{
class odbc_imp;

/// Offers methods for a odbc connection.
class odbc
{
public:
  /// Returns the odbc version.
  static const version_info get_version_info();

  /// Default constructor.
  /// Initializes the odbc connection using specified threaded mode and buffer size.
  explicit odbc(bool threaded_mode = false, size_t buffer_size = 1024);

  /// Destructor.
  /// Logs off.
  ~odbc();

  /// Returns the datasource connected to or to connect to.
  const std::string datasource() const;

  /// Returns true if we are connected.
  bool is_connected() const;

  /// Logs off.
  /// Returns true if you were connected.
  bool logoff();

  /// Logons to the datasource (shows a connection dialog if
  /// a data::window button is specified).
  /// Returns false if dialog cancelled or logon fails.
  bool logon(const data::window& data = data::window());

  /// Runs the query and returns the rows processed count.
  /// Or -1 in case there was an error.
  long query(const std::string& query);

  /// Runs the query and puts results on the grid.
  /// If empty_results then the grid is cleared first.
  /// Returns number of rows appended.
  long query(
    const std::string& query,
    wxGrid*            grid,
    bool&              stopped,
    bool               empty_results = true);

  /// Runs the query and appends results to the stc.
  /// Returns number of lines added.
  long query(const std::string& query, wxStyledTextCtrl* stc, bool& stopped);

private:
  std::unique_ptr<odbc_imp> m_odbc;
};
}; // namespace wex
#endif
