/******************************************************************************\
* File:          util.h
* Purpose:       Include file for wxextension report utilities
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _EX_REPORT_UTIL_H
#define _EX_REPORT_UTIL_H

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/aui/auibar.h>
#include <wx/aui/auibook.h>
#include <wx/filename.h>

#include <wx/extension/dir.h>
#include <wx/extension/statistics.h>

class wxExFrameWithHistory;
class wxExListViewFile;

/// Adds reporting to wxExDir.
class wxExDirWithReport : public wxExDir
{
public:
  /// SetupTool should already be called.
  /// FindFiles invokes RunTool on all matching files.
  wxExDirWithReport(const wxExTool& tool,
    const wxString& fullpath,
    const wxString& filespec = wxEmptyString);

  /// FindFiles causes each found file to be added as listitem to the listview.
  wxExDirWithReport(wxExListViewFile* listview,
    const wxString& fullpath,
    const wxString& filespec = wxEmptyString);

  /// FindFiles causes all found files to be opened using OpenFile from frame.
  /// Flags are passed on to OpenFile.
  wxExDirWithReport(wxExFrameWithHistory* frame,
    const wxString& fullpath,
    const wxString& filespec,
    long flags = 0);

  /// Gets the statistics.
  const wxExFileNameStatistics& GetStatistics() {return m_Statistics;};
protected:
  virtual void OnFile(const wxString& file);
private:
  wxExFileNameStatistics m_Statistics;
  wxExFrameWithHistory* m_Frame;
  wxExListViewFile* m_ListView;
  const long m_Flags;
  wxExTool m_Tool;
};

/// Offers a find toolbar, containing a find combobox and checkboxes.
/// The find combobox allows you to find in an wxExSTCWithFrame
/// component on the specified wxExFrameWithHistory.
class wxExFindToolBar : public wxAuiToolBar
{
public:
  /// Constructor.
  wxExFindToolBar(wxWindow* parent, wxExFrameWithHistory* frame, wxWindowID id = wxID_ANY);
protected:
  void OnCommand(wxCommandEvent& event);
private:
  wxCheckBox* m_RegularExpression;
  wxCheckBox* m_MatchCase;
  wxCheckBox* m_MatchWholeWord;

  DECLARE_EVENT_TABLE()
};

/*! \file */

/// Compares the files, using wxExecute on comparator set in the config.
bool wxExCompareFile(const wxFileName& file1, const wxFileName& file2);

/// Shows a find in files dialog and finds or replaces text in files if chosen.
void wxExFindInFiles(wxExFrameWithHistory* frame, bool replace = false);

/// Finds other filenames from the one specified in the same dir structure.
/// Results are put on the list if not null, or in the filename if not null.
bool wxExFindOtherFileName(
  const wxFileName& filename,
  wxExListViewFile* listview,
  wxFileName* lastfile); // in case more files found, only most recent here

/// Do something (id) for all pages on the notebook.
bool wxExForEach(wxAuiNotebook* notebook, int id, const wxFont& font = wxFont());

/// Opens files and updates history, both for files and projects.
void wxExOpenFiles(wxExFrameWithHistory* frame,
  const wxArrayString& files,
  long flags = 0);
#endif
