/******************************************************************************\
* File:          listitem.h
* Purpose:       Declaration of class 'exListItemWithFileName'
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
#include <wx/extension/textfile.h> // for exRCS

class exListViewFile;

/// Offers a list item associated with a file on an exListView.
/// The item is coloured according to
/// the modification time of the file it is associated with.
/// It allows you to run tools on the item and keeps statistics when running.
class exListItemWithFileName : public exListItem
{
  friend class exDirWithReport; // exDirWithReport uses m_Statistics directly.
public:
  /// Constructor.
  exListItemWithFileName(exListView* listview, const int itemnumber);

  /// Constructor.
  exListItemWithFileName(exListView* listview,
    const wxString& fullpath,
    const wxString& filespec = wxEmptyString);

  /// Gets the filename.
  const exFileName& GetFileName() const {return m_Statistics;};

  /// Gets the statistics.
  const exFileNameStatistics& GetStatistics() const {return m_Statistics;};

  /// Inserts the item at index (if -1 at the end of the listview),
  /// and sets all attributes.
  void Insert(long index = -1);

  /// Runs a tool on this item.
  bool Run(const exTool& tool, exListViewFile* listview);

  /// Updates all attributes.
  void Update();

  /// Sets revision list columns with data from specified rcs.
  void UpdateRevisionList(const exRCS& rcs);
private:
  exFileNameStatistics m_Statistics;
  const wxString m_FileSpec;
};
#endif
