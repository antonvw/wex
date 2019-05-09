////////////////////////////////////////////////////////////////////////////////
// Name:      listitem.h
// Purpose:   Declaration of class wex::listitem
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
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
    listitem(listview* listview,
      const path& filename,
      const std::string& filespec = std::string());
      
    // Deletes this item from the listview.
    void erase() {m_ListView->DeleteItem(GetId());};

    /// Returns the file spec.
    const auto file_spec() const {return m_FileSpec;};
    
    /// Returns the filename.
    const auto & get_filename() const {return m_Path;};

    /// Returns the listview.
    auto* get_listview() const {return m_ListView;};
    
    /// Inserts the item at index (if -1 at the end of the listview),
    /// and sets all attributes.
    void insert(long index = -1);

    /// Returns true if this item is readonly (on the listview).
    bool is_readOnly() const {return m_IsReadOnly;};

    /// Logs info about this item.
    std::stringstream log() const;

    /// Sets the item text using column name.
    /// Returns false if text could not be set.
    bool set_item(const std::string& col_name, const std::string& text);

    /// Updates all attributes.
    void update();
  private:
    void SetReadOnly(bool readonly);

    // Cannot be a wxListCtrl, as find_column is used from listview,
    // and cannot be const, as it calls insert_item on the list.
    listview* m_ListView;

    const path m_Path;
    const std::string m_FileSpec;
    bool m_IsReadOnly;
  };
};
