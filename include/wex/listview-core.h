////////////////////////////////////////////////////////////////////////////////
// Name:      listview.h
// Purpose:   Declaration of wex::listview and related classes
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <vector>
#include <wx/listctrl.h>

namespace wex
{
  /*! \file */
  /// Sort types.
  enum sort_t
  {
    SORT_KEEP = 1,   ///< keep current order, just resort
    SORT_ASCENDING,  ///< sort ascending
    SORT_DESCENDING, ///< sort descending
    SORT_TOGGLE      ///< toggle sort order
  };

  /// Offers a column to be used in a wxListCtrl. Facilitates sorting.
  class column : public wxListItem
  {
  public:
    /// get_column types.
    enum type_t
    {
      INVALID, ///< illegal col
      INT = 1, ///< integer
      DATE,    ///< date
      FLOAT,   ///< float
      STRING   ///< string
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

  namespace core
  {
    /// Offers interface to listview.
    class listview : public wxListView
    {
    public:
      /// Pure virtual interface.

      /// Appends new columns.
      /// Returns false if appending a column failed.
      virtual bool append_columns(const std::vector<column>& cols) = 0;

      /// Finds next.
      virtual bool
      find_next(const std::string& text, bool find_next = true) = 0;
    };
  }; // namespace core
};   // namespace wex
