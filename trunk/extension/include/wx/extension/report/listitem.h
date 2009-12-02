/******************************************************************************\
* File:          listitem.h
* Purpose:       Declaration of class 'wxExListItemWithFileName'
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
class wxExListItemWithFileName : public wxExListItem
{
  friend class wxExDirWithListView; // wxExDirWithListView uses m_Statistics directly.
public:
  /// Constructor.
  wxExListItemWithFileName(wxExListView* listview, const int itemnumber);

  /// Constructor.
  wxExListItemWithFileName(wxExListView* listview,
    const wxString& fullpath,
    const wxString& filespec = wxEmptyString);

  /// Gets the filename.
  const wxExFileName& GetFileName() const {return m_FileName;};

  /// Gets the statistics.
  const wxExFileStatistics& GetStatistics() const {return m_Statistics;};

  /// Inserts the item at index (if -1 at the end of the listview),
  /// and sets all attributes.
  void Insert(long index = -1);

  /// Returns true if this item is readonly (on the listview).
  bool IsReadOnly() const {return m_IsReadOnly;};

  /// Runs a tool on this item.
  const wxExFileStatistics& Run(const wxExTool& tool);

  /// Updates all attributes.
  void Update();

  /// Sets revision list columns with data from specified rcs.
  void UpdateRevisionList(const wxExRCS& rcs);
private:
  void SetReadOnly(bool readonly);

  wxExFileName m_FileName;
  wxExFileStatistics m_Statistics;
  const wxString m_FileSpec;
  bool m_IsReadOnly;
};
#endif
