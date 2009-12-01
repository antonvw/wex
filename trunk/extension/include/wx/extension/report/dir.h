/******************************************************************************\
* File:          dir.h
* Purpose:       Include file for wxExDirWithListView and wxExDirTool classes
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

class wxExListViewFile;

/// Offers a wxExDir with reporting to a listview.
class wxExDirWithListView : public wxExDir
{
public:
  /// FindFiles causes each found file to be added as listitem to the listview.
  wxExDirWithListView(wxExListViewFile* listview,
    const wxString& fullpath,
    const wxString& filespec = wxEmptyString,
    int flags = wxDIR_DEFAULT);
protected:
  virtual void OnDir(const wxString& dir);
  virtual void OnFile(const wxString& file);
private:
  wxExListViewFile* m_ListView;
};

/// Offers a wxExDir with tool support.
class wxExDirTool : public wxExDir
{
public:
  /// SetupTool should already be called.
  /// FindFiles invokes RunTool on all matching files.
  wxExDirTool(const wxExTool& tool,
    const wxString& fullpath,
    const wxString& filespec = wxEmptyString,
    int flags = wxDIR_DEFAULT);
    
  /// Gets the statistics.
  const wxExFileNameStatistics& GetStatistics() const {return m_Statistics;};
protected:  
  void OnFile(const wxString& file);
private:    
  wxExFileNameStatistics m_Statistics;
  const wxExTool m_Tool;
};

#endif
