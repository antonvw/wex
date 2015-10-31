////////////////////////////////////////////////////////////////////////////////
// Name:      listitem.h
// Purpose:   Declaration of class 'wxExListItem'
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/extension/filename.h>
#include <wx/extension/listview.h>

/// Offers a list item associated with a file on an wxExListView.
class WXDLLIMPEXP_BASE wxExListItem : public wxListItem
{
public:
  /// Constructor.
  wxExListItem(wxExListView* listview, long itemnumber);

  /// Constructor.
  wxExListItem(wxExListView* listview,
    const wxExFileName& filename,
    const wxString& filespec = wxEmptyString);
    
  // Deletes this item from the listview.
  void Delete() {m_ListView->DeleteItem(GetId());};

  /// Returns the filename.
  const auto & GetFileName() const {return m_FileName;};

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
  void SetItem(const wxString& col_name, const wxString& text);

  /// Updates all attributes.
  void Update();
private:
  void SetReadOnly(bool readonly);

  // Cannot be a wxListCtrl, as FindColumn is used from wxExListView,
  // and cannot be const, as it calls InsertItem on the list.
  wxExListView* m_ListView;

  const wxExFileName m_FileName;
  const wxString m_FileSpec;
  bool m_IsReadOnly;
};
