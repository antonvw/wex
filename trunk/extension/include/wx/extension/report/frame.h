/******************************************************************************\
* File:          frame.h
* Purpose:       Include file for wxExFrameWithHistory class
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _EX_REPORT_FRAME_H
#define _EX_REPORT_FRAME_H

#include <wx/filehistory.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/report/defs.h>
#include <wx/extension/report/listview.h> // for wxExListViewStandard::ListType 

class wxExConfigDialog;
class wxExListViewFile;
class wxExListViewWithFrame;
class wxExProcess;

/// Adds file and project history support to wxExManagedFrame.
/// It also updates the title of the frame if you have a focused
/// STC file or listview project.
/// Finally it adds process support and find in files dialog.
class wxExFrameWithHistory : public wxExManagedFrame
{
public:
  /// Constructor.
  /// Default it gives file history support to be used from the file menu.
  /// So you should call UseFileHistory somewhere to set it up.
  /// Default it does not use a recent project file.
  wxExFrameWithHistory(wxWindow* parent,
    wxWindowID id,
    const wxString& title,
    size_t maxFiles = 9,
    size_t maxProjects = 0,
    int style = wxDEFAULT_FRAME_STYLE);

  /// Destructor.
 ~wxExFrameWithHistory();

  /// This method is called to activate a certain listview.
  /// Default it returns NULL.
  virtual wxExListViewStandard* Activate(
    wxExListViewStandard::ListType WXUNUSED(list_type), 
    const wxExLexer* WXUNUSED(lexer) = NULL) {
    return NULL;};

  /// If there is a project somewhere, your implementation should return that one.
  /// Default it returns NULL.
  virtual wxExListViewFile* GetProject() {return NULL;};

  /// Returns the recent opened file.
  // Returning a reference here gives a warning.
  const wxString GetRecentFile() const {
    if (m_FileHistory.GetCount() == 0) return wxEmptyString;
    return m_FileHistory.GetHistoryFile(0);}

  /// Returns the recent opened project.
  // Returning a reference here gives a warning.
  const wxString GetRecentProject() const {
    if (m_ProjectHistory.GetCount() == 0) return wxEmptyString;
    return m_ProjectHistory.GetHistoryFile(0);}

  virtual void OnCommandConfigDialog(
    wxWindowID dialogid,
    int commandid = wxID_APPLY);

  /// Interface from wxExFrame.
  virtual bool OpenFile(
    const wxExFileName& filename,
    int line_number = 0,
    const wxString& match = wxEmptyString,
    long flags = 0);

  /// Shows a config dialog, sets the command 
  /// and returns dialog return code.
  int ProcessConfigDialog(
    wxWindow* parent,
    const wxString& title = _("Select Process")) const;
    
  /// Is a process running.
  bool ProcessIsRunning() const;
  
  /// Is a process selected.
  bool ProcessIsSelected() const;
  
  /// Runs the process.
  /// Outputs to a listview LIST_PROCESS.
  /// Returns true if the process executes.
  bool ProcessRun(const wxString& command = wxEmptyString);

  /// Stops the process.
  /// Return true if process could be stopped, or if it was not running at all.
  bool ProcessStop();

  /// Updates file history.
  void SetRecentFile(const wxString& file);

  /// Updates project history.
  void SetRecentProject(const wxString& project) {
    if (!project.empty()) m_ProjectHistory.AddFileToHistory(project);}

  /// Sets the title using file and project.
  void SetTitle(const wxString& file, const wxString& project);

  /// Adds a recent file menu to specified menu,
  /// and sets the file history to use it.
  void UseFileHistory(wxWindowID id, wxMenu* menu);

  /// Uses specified history list, and adds all elements from file history
  /// to the list.
  void UseFileHistoryList(wxExListView* list);

  /// Adds a recent project menu to specified menu,
  /// and sets the project history to use it.
  void UseProjectHistory(wxWindowID id, wxMenu* menu);
protected:
  // Cleans up all as well.
  void OnClose(wxCloseEvent& event);
  void OnCommand(wxCommandEvent& event);
  void OnIdle(wxIdleEvent& event);
private:
  void DoRecent(wxFileHistory& history, int index, long flags = 0);
  void FindInFiles(wxWindowID dialogid);
  /// Shows a find in files dialog and finds or replaces text in files if chosen.
  int FindInFilesDialog(int id = ID_FIND_IN_FILES);
  void UseHistory(wxWindowID id, wxMenu* menu, wxFileHistory& history);

  wxExConfigDialog* m_FiFDialog;
  wxFileHistory m_FileHistory;
  wxExListView* m_FileHistoryList;
  wxFileHistory m_ProjectHistory;
  wxExProcess* m_Process;

  const wxString m_TextInFiles;
  const wxString m_TextInFolder;

  DECLARE_EVENT_TABLE()
};
#endif
