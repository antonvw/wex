/******************************************************************************\
* File:          listview.h
* Purpose:       Declaration of class 'wxExListViewWithFrame'
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _EX_REPORT_LISTVIEW_H
#define _EX_REPORT_LISTVIEW_H

#include <wx/extension/listview.h>
#include <wx/extension/tool.h>

class wxExFrameWithHistory;

/// Combines wxExListView and wxExFile, giving you a list control with file
/// synchronization support. Further it adds some standard lists.
class wxExListViewFile : public wxExListView, public wxExFile
{
public:
  /// The supported lists.
  enum ListType
  {
    LIST_BEFORE_FIRST, ///< for iterating
    LIST_COUNT,        ///< a list to show statistics
    LIST_FIND,         ///< a list to show find results
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
  wxExListViewFile(wxWindow* parent,
    ListType type,
    wxWindowID id = wxID_ANY,
    long menu_flags = LIST_MENU_DEFAULT,
    const wxExLexer* lexer = NULL,
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    long style = wxLC_LIST  | wxLC_HRULES | wxLC_VRULES | wxSUNKEN_BORDER,
    const wxValidator& validator = wxDefaultValidator);

  /// Constructor for a LIST_PROJECT, opens the file.
  wxExListViewFile(wxWindow* parent,
    const wxString& file,
    const wxString& wildcard,
    wxWindowID id = wxID_ANY,
    long menu_flags = LIST_MENU_DEFAULT,
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    long style = wxLC_LIST  | wxLC_HRULES | wxLC_VRULES | wxSUNKEN_BORDER,
    const wxValidator& validator = wxDefaultValidator);

  /// Returns a print header depending on whether we have an associated file
  /// or the type description.
  virtual const wxString PrintHeader() const;

  /// Invokes FileNew and clears the list.
  virtual bool FileNew(const wxExFileName& filename = wxExFileName());

  /// Opens the file and gets all data as list items.
  virtual bool FileOpen(const wxExFileName& filename);

  /// Saves list items to file.
  virtual bool FileSave();

  /// Returns member.
  virtual bool GetContentsChanged() {return m_ContentsChanged;};

  /// Resets the member.
  virtual void ResetContentsChanged() {m_ContentsChanged = false;};

  // Interface, for wxExListView overriden methods.
  /// Sets contents changed if we are not syncing.
  virtual void AfterSorting();

  /// Updates all items.
  virtual void ItemsUpdate();

  /// Tries to insert items from specified text.
  /// Returns true if successfull.
  virtual bool ItemFromText(const wxString& text);

  /// Returns colunm text for specified item.
  virtual const wxString ItemToText(int item_number);

  /// Gets the menu flags.
  long GetMenuFlags() const {return m_MenuFlags;}

  /// Gets the list type.
  const ListType GetType() const {return m_Type;};

  /// Gets the list type as a string.
  const wxString GetTypeDescription() const {return GetTypeDescription(m_Type);};

  /// Gets the list type as a string for specified type.
  static const wxString GetTypeDescription(ListType type);

  /// Returns list type from tool id.
  static ListType GetTypeTool(const wxExTool& tool);
protected:
  virtual void BuildPopupMenu(wxExMenu& menu);
  void OnCommand(wxCommandEvent& event);
  void OnIdle(wxIdleEvent& event);
  void OnList(wxListEvent& event);
  void OnMouse(wxMouseEvent& event);
private:
  void AddItems();
  void Initialize(const wxExLexer* lexer);

  bool m_ContentsChanged;
  bool m_ItemUpdated;
  int m_ItemNumber;
  const long m_MenuFlags;
  const ListType m_Type;

  DECLARE_EVENT_TABLE()
};

/// Adds a wxExFrameWithHistory to wxExListViewFile.
/// This frame member is initialized in the contructor, your main window
/// should be castable to such a frame. It also adds a tool menu if appropriate.
class wxExListViewWithFrame : public wxExListViewFile
{
public:
  /// Constructor.
  wxExListViewWithFrame(wxWindow* parent,
    ListType type,
    wxWindowID id = wxID_ANY,
    long menu_flags = LIST_MENU_DEFAULT,
    const wxExLexer* lexer = NULL,
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    long style = wxLC_LIST  | wxLC_HRULES | wxLC_VRULES | wxSUNKEN_BORDER,
    const wxValidator& validator = wxDefaultValidator);

  /// Constructor for a LIST_PROJECT, opens the file.
  wxExListViewWithFrame(wxWindow* parent,
    const wxString& file,
    const wxString& wildcard,
    wxWindowID id = wxID_ANY,
    long menu_flags = LIST_MENU_DEFAULT,
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    long style = wxLC_LIST  | wxLC_HRULES | wxLC_VRULES | wxSUNKEN_BORDER,
    const wxValidator& validator = wxDefaultValidator);

  /// Opens the file and updates recent project from frame.
  virtual bool FileOpen(const wxExFileName& filename);
protected:
  virtual void BuildPopupMenu(wxExMenu& menu);
  void OnCommand(wxCommandEvent& event);
  void OnList(wxListEvent& event);
private:
  void DeleteDoubles();
  const wxString GetFindInCaption(int id); // cannot be const
  void Initialize();
  void ItemActivated(int item_number);
  void RunItems(const wxExTool& tool);
  wxExFrameWithHistory* m_Frame;

  DECLARE_EVENT_TABLE()
};
#endif
