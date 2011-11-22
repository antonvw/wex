////////////////////////////////////////////////////////////////////////////////
// Name:      listitem.h
// Purpose:   Declaration of class 'wxExListItem'
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EX_REPORT_LISTITEM_H
#define _EX_REPORT_LISTITEM_H

#include <wx/extension/listview.h>
#include <wx/extension/statistics.h>

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

  /// Gets the filename.
  const wxExFileName& GetFileName() const {return m_FileName;};

  /// Gets the file spec.
  const wxString GetFileSpec() const {return m_FileSpec;};
  
  /// Gets the listview.
  wxExListView* GetListView() const {return m_ListView;};
  
  /// Inserts the item at index (if -1 at the end of the listview),
  /// and sets all attributes.
  void Insert(long index = -1);

  /// Returns true if this item is readonly (on the listview).
  bool IsReadOnly() const {return m_IsReadOnly;};

  /// Sets the item text using column number.
  void SetItem(int col_number, const wxString& text);

  /// Sets the item text using column name.
  void SetItem(const wxString& col_name, const wxString& text) {
    SetItem(m_ListView->FindColumn(col_name), text);};

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
#endif
