////////////////////////////////////////////////////////////////////////////////
// Name:      frame.h
// Purpose:   Include file for wxExFrameWithHistory class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2013 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EX_REPORT_FRAME_H
#define _EX_REPORT_FRAME_H

#include <wx/filehistory.h>
#include <wx/extension/listview.h> // for wxExListViewFileName::wxExListType 
#include <wx/extension/managedframe.h>
#include <wx/extension/report/defs.h>

class wxExConfigDialog;
class wxExListView;
class wxExListViewFile;

/// Adds file and project history support to wxExManagedFrame.
/// It also sets a change indicator in the title of the frame if applicable.
/// Finally it adds find in files and selection dialogs.
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

  /// Finds (or replaces) in specified files.
  /// Returns false if dialog was shown, and cancelled,
  /// or SetupTool returned false.
  bool FindInFiles(
    /// the files
    const std::vector< wxString > & files,
    /// ID_TOOL_REPORT_FIND or ID_TOOL_REPORT_REPLACE
    int id,
    /// Default shows a dialog.
    bool show_dialog = true,
    /// report for output
    wxExListView* report = NULL);

  /// Shows a modal find (or replace) in files dialog.
  /// Returns result from ShowModal.
  int FindInFilesDialog(
    /// ID_TOOL_REPORT_FIND or ID_TOOL_REPORT_REPLACE
    int id,
    /// add file types selection as well
    bool add_in_files = false);
  
  /// Returns caption for FindInFilesDialog.
  const wxString GetFindInCaption(int id) const;
  
  /// If there is a project somewhere, 
  /// your implementation should return that one.
  /// Default it returns NULL.
  virtual wxExListViewFile* GetProject() {return NULL;};

  /// Returns the recent opened file.
  // Returning a reference here gives a warning.
  const wxString GetRecentFile() const {
    if (m_FileHistory.GetCount() == 0) return wxEmptyString;
    return m_FileHistory.GetHistoryFile(0);}

  /// Returns file history.
  const wxFileHistory& GetFileHistory() const {return m_FileHistory;};
  
  /// Returns the recent opened project.
  // Returning a reference here gives a warning.
  const wxString GetRecentProject() const {
    if (m_ProjectHistory.GetCount() == 0) return wxEmptyString;
    return m_ProjectHistory.GetHistoryFile(0);}

  /// Override OnCommandConfigDialog for add, find and replace in files.
  virtual void OnCommandConfigDialog(
    wxWindowID dialogid,
    int commandid = wxID_APPLY);
    
  /// Called if the notebook changed page.
  /// Adds page as recently used (file or project, depending on dynamic cast).
  virtual void OnNotebook(wxWindowID id, wxWindow* page);

  /// Interface from wxExFrame.
  virtual bool OpenFile(
    const wxExFileName& filename,
    int line_number = 0,
    const wxString& match = wxEmptyString,
    int col_number = 0,
    long flags = 0);

  /// Updates file history.
  /// Returns true if history was updated.
  bool SetRecentFile(const wxString& file);

  /// Updates project history.
  /// Returns true if history was updated.
  bool SetRecentProject(const wxString& project);

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
  /// Access tp file history list, 
  /// if you use this as a page in a notebook,
  // you might want prevent closing it.
  wxExListView* GetFileHistoryList() {return m_FileHistoryList;};
  
  // Cleans up all as well.
  void OnClose(wxCloseEvent& event);
  void OnCommand(wxCommandEvent& event);
  void OnIdle(wxIdleEvent& event);
private:
  void ClearFileHistory();
  void CreateDialogs();
  void DoRecent(wxFileHistory& history, size_t index, long flags = 0);
  void FindInFiles(wxWindowID dialogid);
  void UseHistory(wxWindowID id, wxMenu* menu, wxFileHistory& history);

  wxExConfigDialog* m_FiFDialog;
  wxExConfigDialog* m_RiFDialog;
  wxFileHistory m_FileHistory;
  wxExListView* m_FileHistoryList;
  wxFileHistory m_ProjectHistory;

  const wxString m_TextInFiles;
  const wxString m_TextInFolder;
  const wxString m_TextRecursive;
  
  bool m_ProjectModified;

  DECLARE_EVENT_TABLE()
};
#endif
