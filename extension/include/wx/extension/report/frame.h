////////////////////////////////////////////////////////////////////////////////
// Name:      frame.h
// Purpose:   Include file for wxExFrameWithHistory class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2012 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EX_REPORT_FRAME_H
#define _EX_REPORT_FRAME_H

#include <wx/filehistory.h>
#include <wx/extension/listview.h> // for wxExListViewFileName::wxExListType 
#include <wx/extension/managedframe.h>
#include <wx/extension/report/defs.h>
#include <wx/extension/report/process.h>

class wxExConfigDialog;
class wxExListView;
class wxExListViewFile;

/// Adds file and project history support to wxExManagedFrame.
/// It also sets a change indicator in the title of the frame if applicable.
/// Finally it adds process support and find in files and selection dialogs.
class WXDLLIMPEXP_BASE wxExFrameWithHistory : public wxExManagedFrame
{
public:
  /// Extra open flags.
  enum
  {
    WIN_IS_PROJECT = 0x0100 ///< open as project
  };

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
  virtual wxExListViewFileName* Activate(
    wxExListViewFileName::wxExListType WXUNUSED(list_type), 
    const wxExLexer* WXUNUSED(lexer) = NULL) {
    return NULL;};
    
  /// Shows a file history popup menu.
  void FileHistoryPopupMenu();

  /// Finds in selected files.  
  bool FindInSelection(
    /// the selection
    const wxArrayString& files,
    /// ID_TOOL_REPORT_FIND or ID_TOOL_REPORT_REPLACE
    int id,
    /// Default shows a dialog.
    bool show_dialog = true,
    /// report for output
    wxExListView* report = NULL);

  /// Finds in selected files dialog.
  int FindInSelectionDialog(
    /// ID_TOOL_REPORT_FIND or ID_TOOL_REPORT_REPLACE
    int id,
    /// add file types selection as well
    bool add_in_files = false);
  
  /// Returns caption for FindInSelectionDialog.
  const wxString GetFindInCaption(int id) const;
  
  /// Gets the process.
  wxExProcess* GetProcess() {return m_Process;};
  
  /// If there is a project somewhere, 
  /// your implementation should return that one.
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

  /// Override OnCommandConfigDialog for add, find and replace in files.
  virtual void OnCommandConfigDialog(
    wxWindowID dialogid,
    int commandid = wxID_APPLY);

  /// Interface from wxExFrame.
  virtual bool OpenFile(
    const wxExFileName& filename,
    int line_number = 0,
    const wxString& match = wxEmptyString,
    long flags = 0);

  /// Updates file history.
  void SetRecentFile(const wxString& file);

  /// Updates project history.
  void SetRecentProject(const wxString& project);

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
  void CreateDialogs();
  void DoRecent(wxFileHistory& history, size_t index, long flags = 0);
  void FindInFiles(wxWindowID dialogid);
  void UseHistory(wxWindowID id, wxMenu* menu, wxFileHistory& history);

  wxExConfigDialog* m_FiFDialog;
  wxExConfigDialog* m_RiFDialog;
  wxFileHistory m_FileHistory;
  wxExListView* m_FileHistoryList;
  wxFileHistory m_ProjectHistory;
  wxExProcessListView* m_Process;

  const wxString m_TextInFiles;
  const wxString m_TextInFolder;
  const wxString m_TextRecursive;

  DECLARE_EVENT_TABLE()
};
#endif
