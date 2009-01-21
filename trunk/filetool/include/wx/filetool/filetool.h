/******************************************************************************\
* File:          filetool.h
* Purpose:       Include file for all wxWidgets filetool classes
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _WXFILETOOL_H
#define _WXFILETOOL_H

#include <wx/extension/extension.h>
#include <wx/extension/dir.h>
#include <wx/filetool/defs.h>
#include <wx/filetool/listview.h>
#include <wx/filetool/listitem.h>
#include <wx/filetool/textfile.h>
#include <wx/filetool/stc.h>
#include <wx/filetool/frame.h>
#include <wx/filetool/version.h>

/// Adds frame and listview interface to exDir. 
class ftDir : public exDir
{
public:
  /// When using FindFiles each found file is added as listitem to the listview.
  /// When using RunTool findfiles is also used to get all matching files,
  /// and on these files RunTool is invoked.
  ftDir(ftListView* listview, 
    const wxString& fullpath, 
    const wxString& filespec = wxEmptyString);

  /// Allows you to FindFiles on a frame.
  /// In the findfiles all found files are opened using OpenFile from frame.
  ftDir(ftFrame* frame, 
    const wxString& fullpath, 
    const wxString& filespec, 
    long flags = 0);

  /// Calls RunTool from exTextFile on all matching files.
  /// SetupTool from exTextFile should already be called.
  size_t RunTool(int flags = wxDIR_DEFAULT);

  /// Gets the statistics.
  const exFileNameStatistics& GetStatistics() {return m_Statistics;};
protected:
  virtual bool Cancelled();
  virtual void OnFile(const wxString& file);
private:
  exFileNameStatistics m_Statistics;
  ftFrame* m_Frame;
  ftListView* m_ListView;
  const long m_Flags;
  bool m_RunningTool;
};

/// Offers a find combobox that allows yuo to find text 
/// on a current STC on an ftFrame.
class ftFind : public wxComboBox
{
public:
  /// Constructor. Fills the combobox box with values from FindReplace from config.
  ftFind(
    wxWindow* parent,
    ftFrame* frame,
    wxWindowID id = wxID_ANY,
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize);
private:
  void OnCommand(wxCommandEvent& event);
  void OnKey(wxKeyEvent& event);
  ftFrame* m_Frame;

  DECLARE_EVENT_TABLE()
};

/*! \file */
// General tool methods.

/// Compares the files, using wxExecute on comparator set in the config.
bool ftCompareFile(const wxFileName& file1, const wxFileName& file2);

/// Shows a find in files dialog and finds or replaces text in files if chosen.
void ftFindInFiles(ftFrame* frame, bool replace = false);

/// Logs about finding/replacing.
void ftFindLog(bool replace);

/// Finds other filenames from the one specified in the same dir structure.
/// Results are put on the list if not null, or in the filename if not null.
bool ftFindOtherFileName(
  const wxFileName& filename, 
  ftListView* listview, 
  wxFileName* lastfile); // in case more files found, only most recent here

/// Do something (id) for all pages on the notebook.
bool ftForEach(wxAuiNotebook* notebook, int id, const wxFont& font = wxFont());

/// Opens files and updates history, both for files and projects.
void ftOpenFiles(ftFrame* frame, 
  const wxArrayString& files, 
  long flags = 0);
#endif
