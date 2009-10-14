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

#include <wx/docview.h> // for wxFileHistory
#include <wx/extension/frame.h>
#include <wx/extension/report/listview.h> // for wxExListViewFile::ListType 

class wxExListViewWithFrame;
class wxExProcessWithListView;
class wxExSTCWithFrame;

/// Adds file and project history support to wxExManagedFrame.
/// It also updates the title of the frame if you have a focused
/// STC file or listview project.
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

  /// This method is called to activate a certain listview.
  /// Default it returns NULL.
  virtual wxExListViewWithFrame* Activate(
    wxExListViewFile::ListType WXUNUSED(list_type), 
    const wxExLexer* WXUNUSED(lexer) = NULL) {
    return NULL;};

  /// If there is a project somewhere, your implementation should return that one.
  /// Default it invokes GetFocusedListView.
  virtual wxExListViewWithFrame* GetProject();

  /// Returns the recent opened file.
  const wxString GetRecentFile() const {
    if (m_FileHistory.GetCount() == 0) return wxEmptyString;
    return m_FileHistory.GetHistoryFile(0);}

  /// Returns the recent opened project.
  const wxString GetRecentProject() const {
    if (m_ProjectHistory.GetCount() == 0) return wxEmptyString;
    return m_ProjectHistory.GetHistoryFile(0);}

  /// Allows you to open a filename with specified contents.
  /// The unique argument can be used as addition for a key in the notebook.
  virtual bool OpenFile(
    const wxExFileName& WXUNUSED(filename),
    const wxString& WXUNUSED(unique),
    const wxString& WXUNUSED(contents)) {return false;};

  /// Interface from wxExFrame.
  virtual bool OpenFile(
    const wxExFileName& filename,
    int line_number = 0,
    const wxString& match = wxEmptyString,
    long flags = 0);

  /// Returns true if a process is running.
  bool ProcessIsRunning() const;

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
  void UseFileHistoryList(wxExListViewWithFrame* list);

  /// Adds a recent project menu to specified menu,
  /// and sets the project history to use it.
  void UseProjectHistory(wxWindowID id, wxMenu* menu);
protected:
  // Cleans up all as well.
  void OnClose(wxCloseEvent& event);
  void OnCommand(wxCommandEvent& event);
  void OnIdle(wxIdleEvent& event);
  void OnUpdateUI(wxUpdateUIEvent& event);
private:
  void DoRecent(wxFileHistory& history, int index, long flags = 0);
  void UseHistory(wxWindowID id, wxMenu* menu, wxFileHistory& history);

  wxFileHistory m_FileHistory;
  wxExListViewWithFrame* m_FileHistoryList;
  wxFileHistory m_ProjectHistory;
  wxExProcessWithListView* m_Process;

  DECLARE_EVENT_TABLE()
};
#endif
