////////////////////////////////////////////////////////////////////////////////
// Name:      listitem.h
// Purpose:   Declaration of class wxExListItem
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/extension/listview.h>
#include <wx/extension/path.h>

/// Offers a list item associated with a file on an wxExListView.
class WXDLLIMPEXP_BASE wxExListItem : public wxListItem
{
public:
  /// Constructor.
  wxExListItem(wxExListView* listview, long itemnumber);

  /// Constructor.
  wxExListItem(wxExListView* listview,
    const wxExPath& filename,
    const std::string& filespec = std::string());
    
  // Deletes this item from the listview.
  void Delete() {m_ListView->DeleteItem(GetId());};

  /// Returns the filename.
  const auto & GetFileName() const {return m_Path;};

  /// Returns the file spec.
  const auto GetFileSpec() const {return m_FileSpec;};
  
  /// Returns the listview.
  auto* GetListView() const {return m_ListView;};
  
  /// Inserts the item at index (if -1 at the end of the listview),
  /// and sets all attributes.
  void Insert(long index = -1);

  /// Returns true if this item is readonly (on the listview).
  bool IsReadOnly() const {return m_IsReadOnly;};

  /// Sets the item text using column name.
  /// Returns false if text could not be set.
  bool SetItem(const std::string& col_name, const std::string& text);

  /// Updates all attributes.
  void Update();
private:
  void SetReadOnly(bool readonly);

  // Cannot be a wxListCtrl, as FindColumn is used from wxExListView,
  // and cannot be const, as it calls InsertItem on the list.
  wxExListView* m_ListView;

  const wxExPath m_Path;
  const std::string m_FileSpec;
  bool m_IsReadOnly;
};
