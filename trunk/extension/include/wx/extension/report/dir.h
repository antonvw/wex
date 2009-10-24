/******************************************************************************\
* File:          dir.h
* Purpose:       Include file for wxExDirWithListView class
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _EX_REPORT_DIR_H
#define _EX_REPORT_DIR_H

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/extension/dir.h>
#include <wx/extension/statistics.h>

class wxExFrame;
class wxExListViewFile;

/// Offers a wxExDir with reporting to a listview.
class wxExDirWithListView : public wxExDir
{
public:
  /// SetupTool should already be called.
  /// FindFiles invokes RunTool on all matching files.
  wxExDirWithListView(const wxExTool& tool,
    const wxString& fullpath,
    const wxString& filespec = wxEmptyString,
    int flags = wxDIR_DEFAULT);

  /// FindFiles causes each found file to be added as listitem to the listview.
  wxExDirWithListView(wxExListViewFile* listview,
    const wxString& fullpath,
    const wxString& filespec = wxEmptyString,
    int flags = wxDIR_DEFAULT);

  /// FindFiles causes all found files to be opened using OpenFile from frame.
  /// Flags are passed on to OpenFile, and dir flags for treating subdirs.
  wxExDirWithListView(wxExFrame* frame,
    const wxString& fullpath,
    const wxString& filespec,
    long file_flags = 0,
    int dir_flags = wxDIR_DEFAULT);

  /// Gets the statistics.
  const wxExFileNameStatistics& GetStatistics() const {return m_Statistics;};
protected:
  virtual void OnDir(const wxString& dir);
  virtual void OnFile(const wxString& file);
private:
  wxExFileNameStatistics m_Statistics;
  wxExFrame* m_Frame;
  wxExListViewFile* m_ListView;
  const long m_Flags;
  wxExTool m_Tool;
};

#endif
