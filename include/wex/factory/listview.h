////////////////////////////////////////////////////////////////////////////////
// Name:      listview.h
// Purpose:   Declaration of wex::factory::listview and related classes
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/factory/control.h>
#include <wex/factory/window.h>
#include <wx/listctrl.h>

#include <string>
#include <vector>

namespace wex
{
/*! \file */
/// Sort types.
enum sort_t
{
  SORT_TOGGLE = 0, ///< toggle sort order
  SORT_KEEP,       ///< keep current order, just resort
  SORT_ASCENDING,  ///< sort ascending
  SORT_DESCENDING, ///< sort descending
};

/// Offers a column to be used in a wxListCtrl. Facilitates sorting.
class column : public wxListItem
{
public:
  /// get_column types.
  enum type_t
  {
    INVALID,       ///< illegal col
    INT = 1,       ///< integer
    DATE,          ///< date
    FLOAT,         ///< float
    STRING_SMALL,  ///< string small size
    STRING_MEDIUM, ///< string medium size
    STRING_LARGE,  ///< string large size
  };

  /// Default constructor.
  column();

  /// Constructor.
  column(
    /// name of the column
    const std::string& name,
    /// type of the column
    type_t type = INT,
    /// width of the column, default width (0) uses a width
    /// that depends on the column type
    /// if you specify a width other than 0, that one is used.
    int width = 0);

  /// Returns whether sorting is ascending.
  bool is_sorted_ascending() const { return m_is_sorted_ascending; }

  /// Sets the sort ascending member.
  void set_is_sorted_ascending(sort_t type);

  /// Returns the column type.
  auto type() const { return m_type; }

private:
  const type_t m_type                = INVALID;
  bool         m_is_sorted_ascending = false;
};

namespace factory
{
/// Offers interface to listview.
class listview : public wxListView
{
public:
  /// Default constructor.
  explicit listview(
    const data::window&  w = data::window(),
    const data::control& c = data::control());

  // Virtual interface.

  /// Appends new columns.
  /// Returns false if appending a column failed.
  virtual bool append_columns(const std::vector<column>& cols)
  {
    return false;
  };

  /// Finds next.
  virtual bool find_next(const std::string& text, bool find_next = true)
  {
    return false;
  };

  /// Prints the list.
  virtual void print() { ; }

  /// Previews the list.
  virtual void print_preview() { ; }
};
}; // namespace factory
}; // namespace wex
