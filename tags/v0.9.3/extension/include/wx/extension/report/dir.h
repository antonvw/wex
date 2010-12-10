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

#include <wx/extension/dir.h>
#include <wx/extension/statistics.h>
#include <wx/extension/report/textfile.h> // for wxExFileStatistics
#include <wx/extension/tool.h>

/// Offers a wxExDir with tool support.
/// RunTool is FindFiles invoked on all matching files.
class WXDLLIMPEXP_BASE wxExDirTool : public wxExDir
{
public:
  /// Constructor, provide your tool and a path.
  /// SetupTool should already be called.
  wxExDirTool(const wxExTool& tool,
    const wxString& fullpath,
    const wxString& filespec = wxEmptyString,
    int flags = wxDIR_DEFAULT);
    
  /// Gets the statistics.
  wxExFileStatistics& GetStatistics() {return m_Statistics;};
protected:  
  void OnFile(const wxString& file);
private:    
  wxExFileStatistics m_Statistics;
  const wxExTool m_Tool;
};

class wxExListView;

/// Offers a wxExDir with reporting to a listview.
/// All matching files and folders are added as listitem to the listview.
class wxExDirWithListView : public wxExDir
{
public:
  /// Constructor, provide your listview and a path.
  wxExDirWithListView(wxExListView* listview,
    const wxString& fullpath,
    const wxString& filespec = wxEmptyString,
    int flags = wxDIR_DEFAULT);
protected:
  virtual void OnDir(const wxString& dir);
  virtual void OnFile(const wxString& file);
private:
  wxExListView* m_ListView;
};
#endif
