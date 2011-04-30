/******************************************************************************\
* File:          listitem.h
* Purpose:       Declaration of class 'wxExListItem'
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _EX_REPORT_LISTITEM_H
#define _EX_REPORT_LISTITEM_H

#include <wx/extension/listview.h>
#include <wx/extension/statistics.h>
#include <wx/extension/textfile.h> // for wxExRCS

/// Offers a list item associated with a file on an wxExListView.
/// It allows you to run tools on the item and keeps statistics when running.
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

  /// Inserts the item at index (if -1 at the end of the listview),
  /// and sets all attributes.
  void Insert(long index = -1);

  /// Returns true if this item is readonly (on the listview).
  bool IsReadOnly() const {return m_IsReadOnly;};

  /// Runs a tool on this item.
  const wxExFileStatistics Run(const wxExTool& tool);

  /// Sets the item text using column number.
  void SetItem(int col_number, const wxString& text);

  /// Sets the item text using column name.
  void SetItem(const wxString& col_name, const wxString& text) {
    SetItem(m_ListView->FindColumn(col_name), text);};

  /// Updates all attributes.
  void Update();

  /// Sets revision list columns with data from specified rcs.
  void UpdateRevisionList(const wxExRCS& rcs);
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
