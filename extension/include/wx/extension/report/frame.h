////////////////////////////////////////////////////////////////////////////////
// Name:      frame.h
// Purpose:   Include file for wex::history_frame class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <set>
#include <wx/extension/filehistory.h>
#include <wx/extension/listview.h>
#include <wx/extension/managedframe.h>

namespace wex
{
  class item_dialog;
  class listview;
  class listview_file;

  /// Adds file and project history support to managed_frame.
  /// It also sets a change indicator in the title of the frame if applicable.
  /// Finally it adds find in files and selection dialogs.
  class history_frame : public managed_frame
  {
  public:
    /// Default constructor.
    /// Default it gives file history support to be used from the file menu.
    /// So you should call UseFileHistory somewhere to set it up.
    /// Default it does not use a recent project file.
    history_frame(
      size_t maxFiles = 9,
      size_t maxProjects = 0,
      const window_data& data = window_data().Style(wxDEFAULT_FRAME_STYLE));

    /// This method is called to activate a certain listview.
    /// Default it returns nullptr.
    virtual listview* Activate(
      listview_type, 
      const lexer* lexer = nullptr) {
      return nullptr;};
      
    /// Finds (or replaces) in specified files.
    /// Returns true if process started.
    bool FindInFiles(
      /// the files
      const std::vector< path > & files,
      /// ID_TOOL_REPORT_FIND or ID_TOOL_REPLACE
      int id,
      /// Default shows a dialog.
      bool show_dialog = true,
      /// report for output
      listview* report = nullptr);

    /// Shows a modal find (or replace) in files dialog.
    /// Returns result from ShowModal.
    int FindInFilesDialog(
      /// ID_TOOL_REPORT_FIND or ID_TOOL_REPLACE
      int id,
      /// add file types selection as well
      bool add_in_files = false);
    
    /// Returns caption for FindInFilesDialog.
    const std::string GetFindInCaption(int id) const;
    
    /// If there is a project somewhere, 
    /// your implementation should return that one.
    /// Default it returns nullptr.
    virtual listview_file* GetProject() {return nullptr;};

    /// Returns project history.
    auto& GetProjectHistory() {return m_ProjectHistory;};
    
    /// Greps for text.
    /// The base directory is the directory for the current stc
    /// component, if available.
    /// Returns true if process started.
    bool Grep(
      /// text [extension] [folder]
      const std::string& line,
      /// normally grep does not replace, by setting sed, it can
      bool sed = false);
    
    /// Override OnCommandItemDialog for add, find and replace in files.
    virtual void OnCommandItemDialog(
      wxWindowID dialogid,
      const wxCommandEvent& event) override;
    
    /// Sed (replace in files).
    /// The base directory is the directory for the current stc
    /// component, if available.
    /// Returns true if process started.
    bool Sed(
      /// text replacement [extension] [folder]
      const std::string& line) {return Grep(line, true);};
      
    /// Updates file history.
    virtual void SetRecentFile(const path& path) override;

    /// Updates project history.
    void SetRecentProject(const path& path) {
      m_ProjectHistory.Add(path);};

    /// Uses specified history list, and adds all elements from file history
    /// to the list.
    void UseFileHistoryList(listview* list);
  protected:
    /// Access to file history list, 
    /// if you use this as a page in a notebook,
    /// you might want prevent closing it.
    listview* GetFileHistoryList() {return m_FileHistoryList;};
    
    void OnIdle(wxIdleEvent& event);
  private:
    void FindInFiles(wxWindowID dialogid);
    const wxString GetFindReplaceInfoText(bool replace = false) const;

    item_dialog* m_FiFDialog {nullptr};
    item_dialog* m_RiFDialog {nullptr};
    listview* m_FileHistoryList {nullptr};
    file_history m_ProjectHistory;

    const std::string m_TextInFiles {_("In files")};
    const std::string m_TextInFolder  {_("In folder")};
    const std::string m_TextRecursive {_("Recursive")};

    // This set determines what fields are placed on the Find Files dialogs
    // as a list of checkboxes.
    const std::set < std::string > m_Info;
  };
};
