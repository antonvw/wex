/******************************************************************************\
* File:          util.h
* Purpose:       Include file for wxExtension report utilities
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
  wxExDirWithListView(wxExFrameWithHistory* frame,
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
  wxExFrameWithHistory* m_Frame;
  wxExListViewFile* m_ListView;
  const long m_Flags;
  wxExTool m_Tool;
};

class ComboBox;

/// Offers a find toolbar, containing a find combobox, up and down arrows
/// and checkboxes.
/// The find combobox allows you to find in an wxExSTCWithFrame
/// component on the specified wxExFrameWithHistory.
class wxExFindToolBar : public wxAuiToolBar
{
public:
  /// Constructor.
  wxExFindToolBar(
    wxWindow* parent, 
    wxExFrameWithHistory* frame, 
    wxWindowID id = wxID_ANY);
protected:
  void OnCommand(wxCommandEvent& event);
private:
  wxCheckBox* m_RegularExpression;
  wxCheckBox* m_MatchCase;
  wxCheckBox* m_MatchWholeWord;
  wxExFrameWithHistory* m_Frame;
  ComboBox* m_ComboBox;

  DECLARE_EVENT_TABLE()
};

/*! \file */

/// Compares the files, using wxExecute on comparator set in the config.
bool wxExCompareFile(const wxFileName& file1, const wxFileName& file2);

/// Shows a find in files dialog and finds or replaces text in files if chosen.
void wxExFindInFiles(bool replace = false);

/// Finds other filenames from the one specified in the same dir structure.
/// Results are put on the list if not null, or in the filename if not null.
bool wxExFindOtherFileName(
  const wxFileName& filename,
  wxExListViewFile* listview,
  wxFileName* lastfile); // in case more files found, only most recent here

/// Do something (id) for all pages on the notebook.
bool wxExForEach(wxAuiNotebook* notebook, int id, const wxFont& font = wxFont());

/// Run make on specified makefile.
bool wxExMake(wxExFrameWithHistory* frame, const wxFileName& makefile);

/// Opens files and updates history, both for files and projects.
void wxExOpenFiles(wxExFrameWithHistory* frame,
  const wxArrayString& files,
  long file_flags = 0,
  int dir_flags = wxDIR_DEFAULT);

/// Shows a dialog and opens selected files and updates history, 
/// both for files and projects.
void wxExOpenFilesDialog(wxExFrameWithHistory* frame,
  long style = wxFD_OPEN | wxFD_MULTIPLE | wxFD_CHANGE_DIR,
  const wxString wildcards = wxEmptyString,
  bool ask_for_continue = false);
#endif
