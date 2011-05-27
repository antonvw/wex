/******************************************************************************\
* File:          listview.h
* Purpose:       Declaration of class 'wxExListViewStandard' and 
*                'wxExListViewWithFrame'
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

/// Adds some standard lists.
class WXDLLIMPEXP_BASE wxExListViewStandard : public wxExListView
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
    LIST_FILE,         ///< a list associated with a file
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

    LIST_MENU_DEFAULT = 
      LIST_MENU_REPORT_FIND | 
      LIST_MENU_TOOL
  };

  /// Constructor.
  wxExListViewStandard(wxWindow* parent,
    ListType type,
    wxWindowID id = wxID_ANY,
    const wxExLexer* lexer = NULL,
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    long style = wxLC_LIST  | wxLC_HRULES | wxLC_VRULES | wxSUNKEN_BORDER,
    const wxValidator& validator = wxDefaultValidator,
    const wxString &name = wxListCtrlNameStr);

  /// Deletes double items.
  void DeleteDoubles();

  /// Gets the list type.
  const ListType GetType() const {return m_Type;};

  /// Gets the list type as a string.
  const wxString GetTypeDescription() const {
    return GetTypeDescription(m_Type);};

  /// Gets the list type as a string for specified type.
  static const wxString GetTypeDescription(ListType type);

  /// Returns list type from tool id.
  static ListType GetTypeTool(const wxExTool& tool);

  /// Updates all items.
  virtual void ItemsUpdate();

  /// Tries to insert items from specified text.
  /// Returns true if successfull.
  virtual bool ItemFromText(const wxString& text);

  /// Returns column text for specified item.
  virtual const wxString ItemToText(long item_number) const;
protected:
  virtual void BuildPopupMenu(wxExMenu& menu);
  void OnCommand(wxCommandEvent& event);
  void OnList(wxListEvent& event);
private:
  void Initialize(const wxExLexer* lexer);
  const ListType m_Type;

  DECLARE_EVENT_TABLE()
};

class wxExFrameWithHistory;

/// Adds a wxExFrameWithHistory to wxExListViewStandard.
/// It also adds a tool menu if appropriate.
class WXDLLIMPEXP_BASE wxExListViewWithFrame : public wxExListViewStandard
{
public:
  /// Constructor.
  wxExListViewWithFrame(wxWindow* parent,
    wxExFrameWithHistory* frame,
    ListType type,
    wxWindowID id = wxID_ANY,
    long menu_flags = LIST_MENU_DEFAULT,
    const wxExLexer* lexer = NULL,
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    long style = wxLC_LIST  | wxLC_HRULES | wxLC_VRULES | wxSUNKEN_BORDER,
    const wxValidator& validator = wxDefaultValidator,
    const wxString &name = wxListCtrlNameStr);    
protected:
  virtual void BuildPopupMenu(wxExMenu& menu);
  wxExFrameWithHistory* GetFrame() {return m_Frame;};
  void OnCommand(wxCommandEvent& event);
  void OnList(wxListEvent& event);
private:
  void ItemActivated(long item_number);
  void RunItems(const wxExTool& tool);

  const long m_MenuFlags;
  wxExFrameWithHistory* m_Frame;

  DECLARE_EVENT_TABLE()
};

#endif
