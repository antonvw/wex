/******************************************************************************\
* File:          listview.h
* Purpose:       Declaration of class 'exListViewFile'
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _FTLISTVIEW_H
#define _FTLISTVIEW_H

#include <wx/extension/listview.h>
#include <wx/extension/tool.h>

class exFrameWithHistory;
class exProcessWithListView;
class exToolThread;

/// Combines exListView and exFile, giving you a list control with file
/// synchronization support. Further it adds processing support.
class exListViewFile : public exListView, public exFile
{
public:
  /// The supported lists.
  enum ListType
  {
    LIST_BEFORE_FIRST, ///< for iterating
    LIST_COUNT,        ///< a list to show statistics
    LIST_FIND,         ///< a list to show find results
    LIST_HEADER,       ///< a list to show headers
    LIST_HISTORY,      ///< a list to show history items
    LIST_KEYWORD,      ///< a list to show keywords
    LIST_REPLACE,      ///< a list to show replace results
    LIST_PROCESS,      ///< a list to show process output
    LIST_PROJECT,      ///< a list to show project items
    LIST_REVISION,     ///< a list to show revisions
    LIST_SQL,          ///< a list to show embedded sql
    LIST_VERSION,      ///< a list to show versions
    LIST_AFTER_LAST,   ///< for iterating
  };

  /// Menu flags, they determine how the context menu will appear.
  enum
  {
    LIST_MENU_REPORT_FIND = 0x0001, ///< for adding find and replace in files
    LIST_MENU_TOOL        = 0x0002, ///< for adding tool menu
    LIST_MENU_RBS         = 0x0004, ///< for adding RBS menu item

    /// RBS is not part of default.
    LIST_MENU_DEFAULT = LIST_MENU_REPORT_FIND | LIST_MENU_TOOL,
  };

  /// Constructor.
  exListViewFile(wxWindow* parent,
    ListType type,
    long menu_flags = LIST_MENU_DEFAULT,
    const exLexer* lexer = NULL,
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    long style = wxLC_LIST  | wxLC_HRULES | wxLC_VRULES | wxSUNKEN_BORDER,
    const wxValidator& validator = wxDefaultValidator);

  /// Constructor for a LIST_PROJECT, opens the file.
  exListViewFile(wxWindow* parent,
    const wxString& file,
    const wxString& wildcard,
    long menu_flags = LIST_MENU_DEFAULT,
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    long style = wxLC_LIST  | wxLC_HRULES | wxLC_VRULES | wxSUNKEN_BORDER,
    const wxValidator& validator = wxDefaultValidator);

  /// Returns a print header depending on whether we have an associated file
  /// or the type description.
  virtual const wxString PrintHeader();

  /// Invokes FileNew and clears the list.
  virtual bool FileNew(const exFileName& filename = exFileName());

  /// Opens the file and gets all data as list items.
  virtual bool FileOpen(const exFileName& filename);

  /// Saves list items to file.
  virtual bool FileSave();
  
  /// Returns member.
  virtual bool GetContentsChanged() {return m_ContentsChanged;};

  /// Resets the member.
  virtual void ResetContentsChanged() {m_ContentsChanged = false;};

  // Interface, for exListView overriden methods.
  /// Sets contents changed if we are not syncing.
  virtual void AfterSorting();

  /// Updates all items.
  virtual void ItemsUpdate();

  /// Tries to insert items from specified text.
  /// Returns true if successfull.
  virtual bool ItemFromText(const wxString& text);

  /// Returns colunm text for specified item.
  virtual const wxString ItemToText(int item_number);

  // Called by exFrameWithHistory::OnClose. Not for doxygen.
  static void CleanUp();

  /// Gets the list type.
  const ListType GetType() const {return m_Type;};

  /// Gets the list type as a string.
  const wxString GetTypeDescription() const {return GetTypeDescription(m_Type);};

  /// Gets the list type as a string for specified type.
  static const wxString GetTypeDescription(ListType type);

  /// Returns list type from tool id.
  static int GetTypeTool(const exTool& tool);

  /// Returns true if a process is running.
  static bool ProcessIsRunning();

  /// Runs the process.
  /// Outputs to a listview LIST_PROCESS.
  static void ProcessRun(const wxString& command = wxEmptyString);

  /// Stops all the processes.
  static void ProcessStop();

  // Called after process has finished, not for doxygen.
  static void ProcessTerminated();
protected:
  void BuildPopupMenu(exMenu& menu);
  void OnCommand(wxCommandEvent& event);
  void OnIdle(wxIdleEvent& event);
  void OnList(wxListEvent& event);
  void OnMouse(wxMouseEvent& event);
  void OnTimer(wxTimerEvent& WXUNUSED(event)) {wxWakeUpIdle();};
private:
  int AddItems();
  void DeleteDoubles();
  const wxString GetFindInCaption(int id);
  void Initialize(const exLexer* lexer);
  bool ItemOpenFile(int item_number);
  void RunItems(const exTool& tool);

  static exProcessWithListView* m_Process;
  exToolThread* m_Thread;

  bool m_ContentsChanged;
  bool m_ItemUpdated;
  int m_ItemNumber;
  exFrameWithHistory* m_Frame;
  const long m_MenuFlags;
  const ListType m_Type;

  DECLARE_EVENT_TABLE()
};
#endif
