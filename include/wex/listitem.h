////////////////////////////////////////////////////////////////////////////////
// Name:      listitem.h
// Purpose:   Declaration of class wex::listitem
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018-2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/listview.h>
#include <wex/path.h>

namespace wex
{
/// Offers a list item associated with a file on an wex::listview.
class listitem : public wxListItem
{
public:
  /// Constructor.
  listitem(listview* listview, long itemnumber);

  /// Constructor.
  listitem(
    listview*          listview,
    const path&        filename,
    const std::string& filespec = std::string());

  // Deletes this item from the listview.
  void erase() { m_listview->DeleteItem(GetId()); }

  /// Returns the file spec.
  const auto file_spec() const { return m_file_spec; }

  /// Returns the listview.
  auto* get_listview() const { return m_listview; }

  /// Inserts the item at index (if -1 at the end of the listview),
  /// and sets all attributes.
  void insert(long index = -1);

  /// Returns true if this item is readonly (on the listview).
  bool is_readonly() const { return m_is_readonly; }

  /// Logs info about this item.
  std::stringstream log() const;

  /// Returns the path.
  const auto& path() const { return m_path; }

  /// Sets the item text using column name.
  /// Returns false if text could not be set.
  bool set_item(const std::string& col_name, const std::string& text);

  /// Updates all attributes.
  void update();

private:
  void set_readonly(bool readonly);

  // Cannot be a wxListCtrl, as find_column is used from listview,
  // and cannot be const, as it calls insert_item on the list.
  listview* m_listview;

  const wex::path   m_path;
  const std::string m_file_spec;
  bool              m_is_readonly;
};
}; // namespace wex
